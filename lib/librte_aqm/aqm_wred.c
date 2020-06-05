/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#include <rte_mbuf.h>

#include <rte_branch_prediction.h> /* TODO: Check necessity */

#include "aqm_wred.h"
#include "pkt_desc_ring.h"
#include "rte_aqm_algorithm.h"

struct aqm_wred {
};

size_t aqm_wred_get_memory_size(struct rte_aqm_wred_params *params)
{
}

int aqm_wred_init(struct aqm_wred *wred, struct rte_aqm_wred_params *params)
{
}

int aqm_wred_destroy(struct aqm_wred *wred)
{
}

int aqm_wred_enqueue(struct aqm_wred *wred, struct rte_mbuf *pkt)
{
}

int aqm_wred_dequeue(struct aqm_wred *wred, struct rte_mbuf **pkt)
{
}

int aqm_wred_get_stats(struct aqm_wred *wred, struct rte_aqm_wred_stats *stats)
{
}
