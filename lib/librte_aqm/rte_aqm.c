/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#include <rte_branch_prediction.h> /* TODO: Check necessity */
#include <rte_log.h>
#include <rte_mbuf.h>

#include "aqm_none.h"
#include "aqm_pie.h"
#include "aqm_red.h"
#include "aqm_wred.h"
#include "rte_aqm.h"
#include "rte_aqm_algorithm.h"

struct rte_aqm {
	enum rte_aqm_algorithm algorithm;
};

size_t rte_aqm_get_memory_size(void *params, enum rte_aqm_algorithm algorithm)
{
	size_t memory_size;

	memory_size = sizeof(struct rte_aqm);

	switch (algorithm) {
		case RTE_AQM_NONE:
			memory_size += aqm_none_get_memory_size(params);
			break;

		case RTE_AQM_RED:
			memory_size += aqm_red_get_memory_size(params);
			break;

		case RTE_AQM_WRED:
			memory_size += aqm_wred_get_memory_size(params);
			break;

		case RTE_AQM_PIE:
			memory_size += aqm_pie_get_memory_size(params);
			break;

		default:
			RTE_LOG(ERR, AQM, "%s: unknown algorithm\n", __func__);
			return -1;
	}

	return memory_size;
}

int rte_aqm_init(struct rte_aqm *aqm, void *params,
		 enum rte_aqm_algorithm algorithm)
{
	int ret;

	aqm->algorithm = algorithm;

	switch (aqm->algorithm) {
		case RTE_AQM_NONE:
			ret = aqm_none_init((void *)&aqm[1], params);
			break;

		case RTE_AQM_RED:
			ret = aqm_red_init((void *)&aqm[1], params);
			break;

		case RTE_AQM_WRED:
			ret = aqm_wred_init((void *)&aqm[1], params);
			break;

		case RTE_AQM_PIE:
			ret = aqm_pie_init((void *)&aqm[1], params);
			break;

		default:
			RTE_LOG(ERR, AQM, "%s: unknown algorithm\n", __func__);
			return -1;
	}

	return ret;
}

int rte_aqm_destroy(struct rte_aqm *aqm)
{
	int ret;

	switch (aqm->algorithm) {
		case RTE_AQM_NONE:
			ret = aqm_none_destroy((void *)&aqm[1]);
			break;

		case RTE_AQM_RED:
			ret = aqm_red_destroy((void *)&aqm[1]);
			break;

		case RTE_AQM_WRED:
			ret = aqm_wred_destroy((void *)&aqm[1]);
			break;

		case RTE_AQM_PIE:
			ret = aqm_pie_destroy((void *)&aqm[1]);
			break;

		default:
			RTE_LOG(ERR, AQM, "%s: unknown algorithm\n", __func__);
			return -1;
	}

	return ret;
}

int rte_aqm_enqueue(struct rte_aqm *aqm, struct rte_mbuf *pkt)
{
	int ret;

	switch (aqm->algorithm) {
		case RTE_AQM_NONE:
			ret = aqm_none_enqueue((void *)&aqm[1], pkt);
			break;

		case RTE_AQM_RED:
			ret = aqm_red_enqueue((void *)&aqm[1], pkt);
			break;

		case RTE_AQM_WRED:
			ret = aqm_wred_enqueue((void *)&aqm[1], pkt);
			break;

		case RTE_AQM_PIE:
			ret = aqm_pie_enqueue((void *)&aqm[1], pkt);
			break;

		default:
			RTE_LOG(ERR, AQM, "%s: unknown algorithm\n", __func__);
			return -1;
	}

	return ret;
}

int rte_aqm_dequeue(struct rte_aqm *aqm, struct rte_mbuf **pkt)
{
	int ret;

	switch (aqm->algorithm) {
		case RTE_AQM_NONE:
			ret = aqm_none_dequeue((void *)&aqm[1], pkt);
			break;

		case RTE_AQM_RED:
			ret = aqm_red_dequeue((void *)&aqm[1], pkt);
			break;

		case RTE_AQM_WRED:
			ret = aqm_wred_dequeue((void *)&aqm[1], pkt);
			break;

		case RTE_AQM_PIE:
			ret = aqm_pie_dequeue((void *)&aqm[1], pkt);
			break;

		default:
			RTE_LOG(ERR, AQM, "%s: unknown algorithm\n", __func__);
			return -1;
	}

	return ret;
}

int rte_aqm_get_stats(struct rte_aqm *aqm, void *stats)
{
	int ret;

	switch (aqm->algorithm) {
		case RTE_AQM_NONE:
			ret = aqm_none_get_stats((void *)&aqm[1], stats);
			break;

		case RTE_AQM_RED:
			ret = aqm_red_get_stats((void *)&aqm[1], stats);
			break;

		case RTE_AQM_WRED:
			ret = aqm_wred_get_stats((void *)&aqm[1], stats);
			break;

		case RTE_AQM_PIE:
			ret = aqm_pie_get_stats((void *)&aqm[1], stats);
			break;

		default:
			RTE_LOG(ERR, AQM, "%s: unknown algorithm\n", __func__);
			return -1;
	}

	return ret;
}
