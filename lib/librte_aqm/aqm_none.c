/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#include <rte_mbuf.h>

#include <rte_branch_prediction.h> /* TODO: Check necessity */

#include "aqm_none.h"
#include "pkt_desc_ring.h"
#include "rte_aqm_algorithm.h"

struct aqm_none {
};

size_t aqm_none_get_memory_size(struct rte_aqm_none_params *params)
{
	return pkt_desc_ring_get_memory_size(params->limit);
}

int aqm_none_init(struct aqm_none *none, struct rte_aqm_none_params *params)
{
	return pkt_desc_ring_init((void *)&none[1], params->limit);
}

int aqm_none_destroy(struct aqm_none *none)
{
	return pkt_desc_ring_destroy((void *)&none[1]);
}

int aqm_none_enqueue(struct aqm_none *none, struct rte_mbuf *pkt)
{
	return pkt_desc_ring_enqueue((void *)&none[1], pkt);
}

int aqm_none_dequeue(struct aqm_none *none, struct rte_mbuf **pkt)
{
	return pkt_desc_ring_dequeue((void *)&none[1], pkt);
}

int aqm_none_get_stats(struct aqm_none *none, struct rte_aqm_none_stats *stats)
{
	stats->length_pkts = pkt_desc_ring_get_length_pkts((void *)&none[1]);
	stats->length_bytes = pkt_desc_ring_get_length_bytes((void *)&none[1]);
	stats->delay = pkt_desc_ring_get_delay((void *)&none[1]);

	return 0;
}
