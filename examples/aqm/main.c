/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>

#include <rte_log.h>
#include <rte_aqm.h>
#include <rte_aqm_algorithm.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_lcore.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>
#include <rte_timer.h>

static volatile bool force_quit;

#define RTE_TEST_RX_DESC_DEFAULT 1024
#define RTE_TEST_TX_DESC_DEFAULT 1024
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

#define MAX_PKT_BURST 32
#define MEMPOOL_CACHE_SIZE 256

#define BURST_TX_DRAIN_US 100
#define STATS_UPDATE_MS 100
#define TIMER_MANAGE_MS 10

/* token bucket values */
#define TB_DROP 1
#define TB_PASS 0

#define MAX_OPT_VALUES 8

#define RTE_LOGTYPE_APP RTE_LOGTYPE_USER1

/* used during argument parsing */
#define AQM_RED 0
#define AQM_CODEL 1
#define AQM_PIE 2
#define AQM_FIFO 3

#define AQM_RED_PARAMS 4
#define AQM_CODEL_PARAMS 2
#define AQM_PIE_PARAMS 4
#define AQM_FIFO_PARAMS 0

struct rte_mempool *aqm_pktmbuf_pool = NULL;

static struct rte_eth_dev_tx_buffer *tx_buffer[RTE_MAX_ETHPORTS];
static struct rte_ring *queue;
static void *aqm_memory;

static struct rte_eth_conf default_port_conf = {
	.rxmode = {
		.split_hdr_size = 0,
	},
	.txmode = {
		.mq_mode = ETH_MQ_TX_NONE,
	}
};

struct aqm_port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t dropped;
	struct rte_aqm_stats aqm_stats;
} __rte_cache_aligned;

struct app_params {
	/* token bucket */
	uint64_t tb_rate;
	uint64_t tb_depth;
	uint64_t tb_tokens;

	/* queue management */
	uint64_t qlen_pkts;
	struct rte_aqm_params aqm_params;

	/* port config */
	uint64_t port_a;
	uint64_t port_b;

	/* core config */
	uint64_t lcore_ab_rx;
	uint64_t lcore_ab_tx;
	uint64_t lcore_ba;
};

static uint64_t tb_prev_cycles;
static uint64_t tb_cur_cycles;

static struct app_params def_params = {
	.tb_rate = 12500000,
	.tb_depth = 10000,
	.tb_tokens = 10000,
	.qlen_pkts = 1024,
	.aqm_params = {
		.algorithm = RTE_AQM_FIFO,
		.algorithm_params = NULL,
	}
};

struct aqm_port_statistics port_statistics[RTE_MAX_ETHPORTS];

static void
signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\nSignal %d received, preparing to exit...\n",
					signum);
		force_quit = true;
	}
}

static const char usage[] =
	" <APP PARAMS>																	\n"
	"Mandatory parameters:																\n"
	"	--aqm 																			\n"
	"	--qlen 																			\n"
	"	--aqm_params 																	\n"
	"	--tb 																			\n"
;

static void
app_usage(void)
{
	printf(usage);
}

static int
packet_handler_tb(struct rte_mbuf *pkt) {
	uint64_t tokens_produced;
	uint64_t cycles;

	cycles = rte_get_tsc_hz();

	if (unlikely(pkt->pkt_len > def_params.tb_depth)) {
		return TB_DROP;
	} else {
		if (def_params.tb_tokens < pkt->pkt_len) {
			tb_cur_cycles = rte_rdtsc();
			while ((((tb_cur_cycles - tb_prev_cycles) * def_params.tb_rate) / cycles) +
				def_params.tb_tokens < pkt->pkt_len) {
				tb_cur_cycles = rte_rdtsc();
			}
			tokens_produced = (((tb_cur_cycles - tb_prev_cycles) * def_params.tb_rate) / cycles);
			if (tokens_produced + def_params.tb_tokens > def_params.tb_depth) {
				def_params.tb_tokens = def_params.tb_depth;
			} else {
				def_params.tb_tokens += tokens_produced;
			}

			tb_prev_cycles = tb_cur_cycles;
		}

		def_params.tb_tokens -= pkt->pkt_len;
	}

	return TB_PASS;
}

static void port_stat(uint8_t portid)
{
	FILE *fp;
	fp = fopen("statistics.txt", "a");

	struct rte_aqm_stats *aqm_stats = &port_statistics[portid].aqm_stats;

	rte_aqm_get_stats(aqm_memory, aqm_stats);
	printf("%lu %lu %lu %lu %u\n",
		aqm_stats->bytes_dequeued, aqm_stats->queue_delay,
		aqm_stats->pkts_dropped_enqueue, aqm_stats->pkts_dropped_dequeue,aqm_stats->length_pkts);
	fprintf(fp, "%lu %lu %lu %lu %u\n",
		aqm_stats->bytes_dequeued, aqm_stats->queue_delay,
		aqm_stats->pkts_dropped_enqueue, aqm_stats->pkts_dropped_dequeue,aqm_stats->length_pkts);

	fclose(fp);

	return;
}

static void ab_rx(void)
{
	printf("Forward path RX on lcore %u\n", rte_lcore_id());

	uint8_t rx_port;
	uint32_t nb_rx;
	uint32_t i;
	uint32_t enq_cnt;

	rx_port = def_params.port_a;
	enq_cnt = 0;

	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct rte_mbuf *m;

	while (!force_quit) {
		nb_rx = rte_eth_rx_burst(rx_port, 0, pkts_burst, MAX_PKT_BURST);

		if (likely(nb_rx)) {
			port_statistics[rx_port].rx += nb_rx;

			for (i = 0; i < nb_rx; ++i) {
				m = pkts_burst[i];
				rte_prefetch0(rte_pktmbuf_mtod(m, void *));

				enq_cnt += rte_aqm_enqueue(aqm_memory, m);
			}
		}
	}

	return;
}

static void ab_tx(void)
{
	printf("Forward path TX on lcore %u\n", rte_lcore_id());

	uint8_t tx_port;
	uint16_t n_pkts_dropped;
	uint32_t sent;
	uint32_t n_bytes_dropped;
	uint64_t prev_tsc;
	uint64_t diff_tsc;
	uint64_t cur_tsc;
	uint64_t drain_tsc;

	struct rte_mbuf *pkt;

	tx_port = def_params.port_b;

	prev_tsc = 0;
	drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
				BURST_TX_DRAIN_US;

	while (!force_quit) {
		if (rte_aqm_dequeue(aqm_memory, &pkt, &n_pkts_dropped, &n_bytes_dropped) == 0) {
			if (packet_handler_tb(pkt) == 0) {
				sent = rte_eth_tx_buffer(tx_port, 0, tx_buffer[tx_port], pkt);
				if (unlikely(sent))
					port_statistics[tx_port].tx += sent;
			}
		}

		cur_tsc = rte_rdtsc();
		diff_tsc = cur_tsc - prev_tsc;

		if (unlikely(diff_tsc > drain_tsc)) {
			sent = rte_eth_tx_buffer_flush(tx_port, 0, tx_buffer[tx_port]);
			if (likely(sent))
				port_statistics[tx_port].tx += sent;
		}
	}

	return;
}

static void ba_rx_tx(void)
{
	printf("Reverse path RX and TX on lcore %u\n", rte_lcore_id());

	uint8_t rx_port;
	uint8_t tx_port;
	uint32_t nb_rx;
	uint32_t sent;
	uint32_t i;
	uint64_t prev_tsc;
	uint64_t diff_tsc;
	uint64_t cur_tsc;
	uint64_t drain_tsc;

	rx_port = def_params.port_b;
	tx_port = def_params.port_a;

	prev_tsc = 0;
	drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
				BURST_TX_DRAIN_US;

	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct rte_mbuf *m;

	while (!force_quit)
	{
		nb_rx = rte_eth_rx_burst(rx_port, 0, pkts_burst, MAX_PKT_BURST);
		
		if (likely(nb_rx)) {
			port_statistics[rx_port].rx += nb_rx;

			for (i = 0; i < nb_rx; ++i) {
				m = pkts_burst[i];
				rte_prefetch0(rte_pktmbuf_mtod(m, void *));
				sent = rte_eth_tx_buffer(tx_port, 0, tx_buffer[tx_port], m);

				if (unlikely(sent))
					port_statistics[tx_port].tx += sent;
			}
		}

		cur_tsc = rte_rdtsc();
		diff_tsc = cur_tsc - prev_tsc;

		if (unlikely(diff_tsc > drain_tsc)) {
			sent = rte_eth_tx_buffer_flush(tx_port, 0, tx_buffer[tx_port]);
			if (likely(sent))
				port_statistics[tx_port].tx += sent;
		}
	}

	return;
}

static int print_stats_manage_timer(void)
{
	printf("Statistics and timer management on lcore %u\n", rte_lcore_id());

	uint64_t stats_prev_tsc;
	uint64_t stats_diff_tsc;
	uint64_t stats_cur_tsc;
	uint64_t stats_tsc;
	uint64_t timer_prev_tsc;
	uint64_t timer_diff_tsc;
	uint64_t timer_cur_tsc;
	uint64_t timer_tsc;

	stats_prev_tsc = 0;
	timer_prev_tsc = 0;
	stats_tsc = (rte_get_tsc_hz() + MS_PER_S - 1) / MS_PER_S *
					STATS_UPDATE_MS;
	timer_tsc = (rte_get_tsc_hz() + MS_PER_S - 1) / MS_PER_S *
					TIMER_MANAGE_MS;

	while (!force_quit)
	{
		stats_cur_tsc = rte_rdtsc();
		stats_diff_tsc = stats_cur_tsc - stats_prev_tsc;

		if (unlikely(stats_diff_tsc > stats_tsc)) {
			port_stat(def_params.port_b);
			stats_prev_tsc = stats_cur_tsc;
		}

		timer_cur_tsc = rte_rdtsc();
		timer_diff_tsc = timer_cur_tsc - timer_prev_tsc;

		if (unlikely(timer_diff_tsc > timer_tsc)) {
			rte_timer_manage();
			timer_prev_tsc = timer_cur_tsc;
		}
	}

	return 0;
}

static int assign_jobs(__rte_unused void *arg)
{
	uint8_t lcore_id;

	lcore_id = rte_lcore_id();

	if (lcore_id == def_params.lcore_ab_rx)
		ab_rx();
	else if (lcore_id == def_params.lcore_ab_tx)
		ab_tx();
	else if (lcore_id == def_params.lcore_ba)
		ba_rx_tx();

	return 0;
}

static int
app_parse_opt_vals(const char *conf_str, char separator, uint32_t n_vals, uint64_t *opt_vals)
{
	char *string;
	int i, n_tokens;
	char *tokens[MAX_OPT_VALUES];

	if (conf_str == NULL || opt_vals == NULL || n_vals == 0 || n_vals > MAX_OPT_VALUES)
		return -1;

	string = strdup(conf_str);
	if (string == NULL)
		return -1;

	n_tokens = rte_strsplit(string, strnlen(string, 32), tokens, n_vals, separator);

	if (n_tokens > MAX_OPT_VALUES)
		return -1;

	for (i = 0; i < n_tokens; i++)
		opt_vals[i] = (uint64_t) atol(tokens[i]);

	free(string);

	return n_tokens;
}

static int
app_parse_tb_config(const char *conf_str)
{
	int ret;
	uint64_t vals[3];

	ret = app_parse_opt_vals(conf_str, ',', 3, vals);
	if (ret != 3)
		return ret;

	def_params.tb_rate = vals[0];
	def_params.tb_depth = vals[1];
	def_params.tb_tokens = vals[2];

	return 0;
}

static int
app_parse_port_config(const char *conf_str)
{
	int ret;
	uint64_t vals[2];

	ret = app_parse_opt_vals(conf_str, ',', 2, vals);
	if (ret != 3)
		return ret;

	def_params.port_a = vals[0];
	def_params.port_b = vals[1];

	return 0;
}

static int
app_parse_core_config(const char *conf_str)
{
	int ret;
	uint64_t vals[3];

	ret = app_parse_opt_vals(conf_str, ',', 3, vals);
	if (ret != 3)
		return ret;

	def_params.lcore_ab_rx = vals[0];
	def_params.lcore_ab_tx = vals[1];
	def_params.lcore_ba = vals[2];

	return 0;
}

static int
app_parse_aqm_config(const char *conf_str, int aqm_algorithm, int num_params)
{
	int ret;
	uint64_t vals[num_params];

	if (aqm_algorithm != 4) {
		ret = app_parse_opt_vals(conf_str, ',', num_params, vals);
		if (ret != num_params)
			return ret;
	}

	switch (aqm_algorithm)
	{
		case AQM_RED:
		{
			static struct rte_red_params red_params;
			red_params.min_th = (uint16_t) vals[0];
			red_params.max_th = (uint16_t) vals[1];
			red_params.maxp_inv = (uint16_t) vals[2];
			red_params.wq_log2 = (uint16_t) vals[3];

			def_params.aqm_params.algorithm = RTE_AQM_RED;
			def_params.aqm_params.algorithm_params = &red_params;
			break;
		}
		case AQM_CODEL:
		{
			static struct rte_codel_params codel_params;
			codel_params.target = (uint32_t) vals[0];
			codel_params.interval = (uint32_t) vals[1];

			def_params.aqm_params.algorithm = RTE_AQM_CODEL;
			def_params.aqm_params.algorithm_params = &codel_params;
			break;
		}
		case AQM_PIE:
		{
			static struct rte_pie_params pie_params;
			pie_params.target_delay = (uint32_t) vals[0];
			pie_params.t_update = (uint32_t) vals[1];
			pie_params.mean_pkt_size = (uint32_t) vals[2];
			pie_params.max_burst = (uint32_t) vals[3];

			def_params.aqm_params.algorithm = RTE_AQM_PIE;
			def_params.aqm_params.algorithm_params = &pie_params;
			break;
		}
		case AQM_FIFO:
		{
			def_params.aqm_params.algorithm = RTE_AQM_FIFO;
			def_params.aqm_params.algorithm_params = NULL;
		}
	}

	return 0;
}

static int parse_args(int argc, char **argv)
{
	static struct option lgopts[] =
	{
		{ "qlen", 1, 0, 0 },
		{ "tb", 1, 0, 0 },
		{ "aqm", 1, 0, 0 },
		{ "aqm_params", 1, 0, 0 },
		{ "port_conf", 1, 0, 0 },
		{ "core_conf", 1, 0, 0 },
		{ NULL, 0, 0, 0 }
	};

	int option_index = 0;
	int opt = 0;
	int aqm_algorithm = -1;
	int num_aqm_params = -1;
	int ret;

	const char *optname;

	while ((opt = getopt_long(argc, argv, "", lgopts, &option_index)) != EOF) {
		switch (opt) {
			case 0:
				optname = lgopts[option_index].name;
				if (strcmp(optname, "qlen") == 0) {
					def_params.qlen_pkts = (uint64_t) atol(optarg);
					if (def_params.qlen_pkts <= 0) {
						RTE_LOG(ERR, APP, "Invalid queue length %s\n", optarg);
						return -1;
					}
					break;
				}
				if (strcmp(optname, "tb") == 0) {
					ret = app_parse_tb_config(optarg);
					if (ret) {
						RTE_LOG(ERR, APP, "Invalid token bucket config\n");
						return -1;
					}
					break;
				}
				if (strcmp(optname, "aqm") == 0) {
					if (strcmp(optarg, "codel") == 0) {
						aqm_algorithm = AQM_CODEL;
						num_aqm_params = AQM_CODEL_PARAMS;
					} else if (strcmp(optarg, "pie") == 0) {
						aqm_algorithm = AQM_PIE;
						num_aqm_params = AQM_PIE_PARAMS;
					} else if (strcmp(optarg, "red") == 0) {
						aqm_algorithm = AQM_RED;
						num_aqm_params = AQM_RED_PARAMS;
					} else if (strcmp(optarg, "fifo") == 0) {
						aqm_algorithm = AQM_FIFO;
						num_aqm_params = AQM_FIFO_PARAMS;
					} else {
						RTE_LOG(ERR, APP, "Unknown AQM type %s\n", optarg);
						return -1;
					}
					break;
				}
				if (strcmp(optname, "aqm_params") == 0) {
					if (aqm_algorithm == -1) {
						RTE_LOG(ERR, APP, "--aqm_params must follow --aqm\n");
						return -1;
					}
					ret = app_parse_aqm_config(optarg, aqm_algorithm, num_aqm_params);
					if (ret) {
						RTE_LOG(ERR, APP, "Invalid AQM config\n");
						return -1;
					}
					break;
				}
				if (strcmp(optname, "port_conf") == 0) {
					ret = app_parse_port_config(optarg);
					if (ret) {
						RTE_LOG(ERR, APP, "Invalid port config\n");
						return -1;
					}
					break;
				}
				if (strcmp(optname, "core_conf") == 0) {
					ret = app_parse_core_config(optarg);
					if (ret) {
						RTE_LOG(ERR, APP, "Invalid core config\n");
						return -1;
					}
					break;
				}
			break;
			default:
				app_usage();
				return -1;
		}
	}
	return 0;
}

static int port_init(uint8_t portid)
{
	int ret = 0;

	struct rte_eth_rxconf rxq_conf;
	struct rte_eth_txconf txq_conf;
	struct rte_eth_conf local_port_conf;
	struct rte_eth_dev_info dev_info;

	local_port_conf = default_port_conf;

	ret = rte_eth_dev_info_get(portid, &dev_info);
	if (ret != 0)
		return ret;

	if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		local_port_conf.txmode.offloads |=
				DEV_TX_OFFLOAD_MBUF_FAST_FREE;

	ret = rte_eth_dev_configure(portid, 1, 1, &local_port_conf);
	if (ret < 0)
		return ret;

	ret = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &nb_rxd,
				&nb_txd);
	if (ret < 0)
		return ret;

	rxq_conf = dev_info.default_rxconf;
	rxq_conf.offloads = local_port_conf.rxmode.offloads;
	ret = rte_eth_rx_queue_setup(portid, 0, nb_rxd,
				rte_eth_dev_socket_id(portid),
				&rxq_conf,
				aqm_pktmbuf_pool);
	if (ret < 0)
		return ret;

	txq_conf = dev_info.default_txconf;
	txq_conf.offloads = local_port_conf.txmode.offloads;
	
	ret = rte_eth_tx_queue_setup(portid, 0, nb_txd,
				rte_eth_dev_socket_id(portid),
				&txq_conf);
	
	if (ret < 0)
		return ret;

	tx_buffer[portid] = rte_zmalloc_socket("tx_buffer",
				RTE_ETH_TX_BUFFER_SIZE(MAX_PKT_BURST), 0,
				rte_eth_dev_socket_id(portid));

	if (tx_buffer[portid] == NULL)
		return -1;

	ret = rte_eth_tx_buffer_init(tx_buffer[portid], MAX_PKT_BURST);
	if (ret)
		return ret;

	ret = rte_eth_tx_buffer_set_err_callback(tx_buffer[portid],
				rte_eth_tx_buffer_count_callback,
				&port_statistics[portid].dropped);
	if (ret)
		return ret;

	ret = rte_eth_dev_set_ptypes(portid, RTE_PTYPE_UNKNOWN, NULL, 0);
	if (ret < 0)
		return ret;

	ret = rte_eth_dev_start(portid);
	if (ret < 0)
		return ret;

	ret = rte_eth_promiscuous_enable(portid);
	if (ret)
		return ret;

	memset(&port_statistics, 0, sizeof(port_statistics));

	return 0;
}

static void
print_def_params(void) {

	printf("qlength = %ld\n", def_params.qlen_pkts);
	printf("tb_rate = %ld\ntb_depth = %ld\ntb_tokens = %ld\n",
		def_params.tb_rate,
		def_params.tb_depth,
		def_params.tb_tokens);
	printf("ports = %ld, %ld\n",
			def_params.port_a,
			def_params.port_b);
	printf("cores = %ld, %ld, %ld\n",
			def_params.lcore_ab_rx,
			def_params.lcore_ab_tx,
			def_params.lcore_ba);
	printf("aqm = ");

	switch(def_params.aqm_params.algorithm) {
		case RTE_AQM_WRED:
			printf("wred\n");
			break;
		case RTE_AQM_FIFO:
			printf("fifo\n");
			break;
		case RTE_AQM_PIE: ;
			struct rte_pie_params *pie_params = 
					(struct rte_pie_params * ) def_params.aqm_params.algorithm_params;
			printf("pie\n");
			printf("target_delay = %d\nt_update = %d\nmean_pkt_size = %d\nmax_burst = %d\n",
				pie_params->target_delay,
				pie_params->t_update,
				pie_params->mean_pkt_size,
				pie_params->max_burst);
			break;
		case RTE_AQM_CODEL: ;
			struct rte_codel_params *codel_params = 
					(struct rte_codel_params *) def_params.aqm_params.algorithm_params;
			printf("codel\n");
			printf("target = %ld\ninterval = %ld\n",
				codel_params->target,
				codel_params->interval);
			break;
		case RTE_AQM_RED: ;
			struct rte_red_params *red_params = 
					(struct rte_red_params *) def_params.aqm_params.algorithm_params;
			printf("red\n");
			printf("min_th = %d\nmax_th = %d\nmaxp_inv = %d\nwq_log2 = %d\n",
				red_params->min_th,
				red_params->max_th,
				red_params->maxp_inv,
				red_params->wq_log2);
			break;
	}
}

int main(int argc, char **argv)
{
	int ret;
	uint16_t nb_ports;
	uint16_t aqm_memory_size;
	uint32_t nb_lcores;
	uint32_t nb_mbufs;

	nb_ports = 2;
	nb_lcores = 5;

	force_quit = false;
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

	ret = parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid application arguments\n");

	nb_mbufs = RTE_MAX(nb_ports * (nb_rxd + nb_txd + MAX_PKT_BURST +
				nb_lcores * MEMPOOL_CACHE_SIZE), 8192U);

	aqm_pktmbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", nb_mbufs,
				MEMPOOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
				rte_socket_id());

	if (aqm_pktmbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

	ret = port_init(def_params.port_a);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Failed to initialize port %ld\n", def_params.port_a);

	ret = port_init(def_params.port_b);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Failed to initialize port %ld\n", def_params.port_b);

	aqm_memory_size = rte_aqm_get_memory_size(def_params.aqm_params.algorithm);

	queue = rte_ring_create("qbase", def_params.qlen_pkts,
				rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ);

	if (queue == NULL)
		rte_exit(EXIT_FAILURE, "Failed to allocate queue memory\n");

	aqm_memory = rte_zmalloc_socket("aqm_memory", aqm_memory_size,
				RTE_CACHE_LINE_SIZE, rte_eth_dev_socket_id(def_params.port_b));

	if (queue == NULL)
		rte_exit(EXIT_FAILURE, "Failed to allocate aqm memory\n");

	if (rte_aqm_init(aqm_memory, &def_params.aqm_params, queue,
				def_params.qlen_pkts) != 0)
		rte_exit(EXIT_FAILURE, "Failed to initialize AQM\n");

	tb_prev_cycles = rte_rdtsc();

	print_def_params();

	rte_eal_mp_remote_launch(assign_jobs, NULL, SKIP_MASTER);

	print_stats_manage_timer();

	rte_eal_mp_wait_lcore();

	rte_aqm_destroy(aqm_memory);
	rte_free(aqm_memory);
	rte_free(queue);

	rte_eth_dev_stop(def_params.port_a);
	rte_eth_dev_stop(def_params.port_b);

	rte_eth_dev_close(def_params.port_a);
	rte_eth_dev_close(def_params.port_b);

	printf("Exiting..\n");
	return 0;
}
