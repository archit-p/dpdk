/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#include <rte_mbuf.h>

#include <rte_branch_prediction.h> /* TODO: Check necessity */

#include "aqm_pie.h"
#include "pkt_desc_ring.h"
#include "rte_aqm_algorithm.h"

#define PIE_DROP 		1				/**< return value: Drop the packet */
#define PIE_ENQUEUE		0				/**< return value: Enqueue the packet */
#define PIE_FIX_POINT_BITS 	13				/**< Number of bits for fractional part */
#define PIE_PROB_BITS	   	31				/**< Length of drop probability in bits */
#define PIE_MAX_PROB	   	((1LL<<PIE_PROB_BITS)-1)	/**< Max drop probability value */
#define PIE_SCALE		(1LL<<PIE_FIX_POINT_BITS)

/**
 * PIE configuration parameters
 */
struct pie_config {
	uint32_t target_delay;		/**< target_delay */
	uint32_t t_update;		/**< t_update */
	uint32_t alpha;			/**< alpha */
	uint32_t beta;			/**< beta */
	uint32_t mean_pkt_size;		/**< mean_pkt_size */
	uint32_t max_burst;		/**< max_burst */
};

/**
 * PIE runtime data
 */
struct pie_rt {
	int64_t drop_prob;		/**< The current packet drop probability. Reset to 0 */
	uint32_t burst_allowance;	/**< Current burst allowance */
	uint64_t old_qdelay;	  	/**< The previous queue delay estimate. Reset to 0 */
	uint64_t cur_qdelay;	  	/**< The current queue delay estimate */
	uint64_t accu_prob;	  	/**< Accumulated drop probability. Reset to 0 */
	struct rte_timer *pie_timer;	/**< A timer to periodically invoke drop probability calculation module. */
};

struct aqm_pie {
	struct pie_rt pie_rt;
	struct pie_config pie_config;
	struct rte_timer pie_timer;
};

static int 
aqm_pie_drop(struct pie_config *config,
	struct pie_rt *pie_rt,
	uint32_t qlen) 
{
	if((pie_rt->old_qdelay < config->target_delay>>1 \
		&& pie_rt->drop_prob < PIE_MAX_PROB/5) \
		|| (qlen <= 2*config->mean_pkt_size)){
		return PIE_ENQUEUE;
	}

	if(pie_rt->drop_prob == 0) {
		pie_rt->accu_prob = 0;
	}
	pie_rt->accu_prob += pie_rt->drop_prob;

	if(pie_rt->accu_prob < (uint64_t) PIE_MAX_PROB * 17 / 20) {
		return PIE_ENQUEUE;
	}

	if(pie_rt->accu_prob >= (uint64_t) PIE_MAX_PROB * 17 / 2) {
		return PIE_DROP;
	}	

	int64_t random = rte_rand() % PIE_MAX_PROB;

	if (random < pie_rt->drop_prob) {
		pie_rt->accu_prob = 0;
		return PIE_DROP;
	}
	
	return PIE_ENQUEUE;
}

__rte_unused static void 
aqm_pie_calc_drop_prob(__attribute__((unused)) struct rte_timer *tim, 
	void *arg)
{
	struct aqm_pie *pie = (struct aqm_pie *) arg;
	struct pie_config *config = &pie->pie_config;
	struct pie_rt *pie_rt = &pie->pie_rt;
	int p_isneg;
	
	uint64_t cur_qdelay = pie_rt->cur_qdelay;
	uint64_t old_qdelay = pie_rt->old_qdelay;
	uint64_t target_delay = config->target_delay;
	int64_t oldprob;
	
	int64_t p = config->alpha * (cur_qdelay - target_delay) + \
		config->beta * (cur_qdelay - old_qdelay);

	p_isneg = p < 0;

	if (p_isneg)
		p = -p;
		
	
	p *= (PIE_MAX_PROB << 12) / US_PER_S;
	// Drop probability auto-tuning logic as per RFC 8033
	if (pie_rt->drop_prob < (PIE_MAX_PROB / 1000000)) {
		p >>= 11 + PIE_FIX_POINT_BITS + 12;
	} else if (pie_rt->drop_prob < (PIE_MAX_PROB / 100000)) {
		p >>= 9 + PIE_FIX_POINT_BITS + 12;
	} else if (pie_rt->drop_prob < (PIE_MAX_PROB / 10000)) {
		p >>= 7 + PIE_FIX_POINT_BITS + 12;
	} else if (pie_rt->drop_prob < (PIE_MAX_PROB / 1000)) {
		p >>= 5 + PIE_FIX_POINT_BITS + 12;
	} else if (pie_rt->drop_prob < (PIE_MAX_PROB / 100)) {
		p >>= 3 + PIE_FIX_POINT_BITS + 12;
	} else if (pie_rt->drop_prob < (PIE_MAX_PROB / 10)) {
		p >>= 1 + PIE_FIX_POINT_BITS + 12;
	} else {
		p >>= PIE_FIX_POINT_BITS + 12;
	}

	oldprob = pie_rt->drop_prob;
	
	if (p_isneg) {
		pie_rt->drop_prob = pie_rt->drop_prob - p;
		if (pie_rt->drop_prob > oldprob) {
			pie_rt->drop_prob = 0;
		}
	} else {
		// Cap Drop Adjustment
		if (pie_rt->drop_prob >= PIE_MAX_PROB / 10 && \
				p > PIE_MAX_PROB / 50 ) {
			p = PIE_MAX_PROB / 50;
		}

		pie_rt->drop_prob += p;

		if (pie_rt->drop_prob < oldprob) {
			pie_rt->drop_prob = PIE_MAX_PROB;
		}
	}

	if (pie_rt->drop_prob < 0) {
		pie_rt->drop_prob = 0;
	} else {
		// Exponentially decay the drop_prob when queue is empty
		if (cur_qdelay == 0 && old_qdelay == 0) {
			pie_rt->drop_prob -= pie_rt->drop_prob >> 6;
		}

		if (pie_rt->drop_prob > PIE_MAX_PROB) {
			pie_rt->drop_prob = PIE_MAX_PROB;
		}
	}

	pie_rt->old_qdelay = cur_qdelay;

	// Update burst allowance
	if (pie_rt->burst_allowance < config->t_update) {
		pie_rt->burst_allowance = 0;
	} else {
		pie_rt->burst_allowance -= config->t_update;
	}
}

size_t aqm_pie_get_memory_size(struct rte_aqm_pie_params *params)
{
}

int aqm_pie_init(struct aqm_pie *pie, struct rte_aqm_pie_params *params)
{
	struct rte_pie_params *pie_params = &params->params;
	struct pie_config *config = &pie->pie_config;
	struct pie_rt *pie_rt = &pie->pie_rt;

	if (config == NULL) {
		return -1;
	}
	if (pie_params->target_delay <= 0) {
		return -2;
	}
	if (pie_params->max_burst <= 0) {
		return -3;
	}
	if (pie_params->t_update <= 0) {
		return -4;
	}
	if (pie_params->mean_pkt_size <= 0) {
		pie_params->mean_pkt_size = 2;
	}

	config->target_delay    = pie_params->target_delay * 1000u;
	config->t_update        = pie_params->t_update * 1000u;
	config->alpha           = PIE_SCALE * 0.125;
	config->beta            = PIE_SCALE * 1.25;
	config->mean_pkt_size   = pie_params->mean_pkt_size;
	config->max_burst 	= pie_params->max_burst * 1000u;

	pie_rt->drop_prob = 0;
	pie_rt->burst_allowance = config->max_burst;
	pie_rt->old_qdelay = 0;
	pie_rt->cur_qdelay = 0;
	pie_rt->accu_prob = 0;

	uint64_t t = (rte_get_tsc_hz()+US_PER_S-1)/US_PER_S * pie->pie_config.t_update;
	unsigned int lcore_id = rte_lcore_id();

	rte_timer_subsystem_init();

	rte_timer_reset(&pie->pie_timer, t, PERIODICAL,
		lcore_id, aqm_pie_calc_drop_prob, (void *) pie);

	return 0;
}

int aqm_pie_destroy(struct aqm_pie *pie)
{
}

int aqm_pie_enqueue(struct aqm_pie *pie, struct rte_mbuf *pkt)
{
	if(pie == NULL)
		return -1;
	
	uint16_t qlen = circular_queue_get_length_pkts(cq);
	struct pie_config *config = &pie->pie_config;
	struct pie_rt *pie_rt = &pie->pie_rt;
	
	if (unlikely(circular_queue_is_full(cq))) {
		pie_rt->accu_prob = 0;
		return 1;
	}

	if (pie_rt->burst_allowance == 0 && aqm_pie_drop(config, pie_rt, qlen) == PIE_DROP) {
		rte_pktmbuf_free(pkt);
		return 1;
	}

	if (pie_rt->drop_prob == 0 \
		&& pie_rt->cur_qdelay < config->target_delay>>1 \
		&& pie_rt->old_qdelay < config->target_delay>>1) {
		pie_rt->burst_allowance = config->max_burst;
	}

	return circular_queue_enqueue(cq, pkt);
}

int aqm_pie_dequeue(struct aqm_pie *pie, struct rte_mbuf **pkt)
{
	int ret = 0;

	ret = circular_queue_dequeue(cq, pkt);
	if (unlikely(ret)) {
		RTE_LOG(ERR, AQM, "%s: dequeue failure\n", __func__);
		return -1;
	}

	*n_pkts_dropped = 0;
	*n_bytes_dropped = 0;

	pie->pie_rt.cur_qdelay = circular_queue_get_queue_delay(cq);
	return 0;
}

int aqm_pie_get_stats(struct aqm_pie *pie, struct rte_aqm_pie_stats *stats)
{
}
