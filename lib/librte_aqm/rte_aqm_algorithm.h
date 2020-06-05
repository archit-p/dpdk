/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _RTE_AQM_ALGORITHM_H_
#define _RTE_AQM_ALGORITHM_H_

/**
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * RTE Active Queue Management (AQM) Algorithm
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum rte_aqm_algorithm {
	RTE_AQM_NONE,
	RTE_AQM_RED,
	RTE_AQM_WRED,
	RTE_AQM_PIE,
	RTE_AQM_CODEL
};

struct rte_aqm_none_params {
	uint16_t limit;
};

struct rte_aqm_none_stats {
	uint64_t delay;
	uint32_t length_bytes;
	uint16_t length_pkts;
};

struct rte_aqm_red_params {
	uint16_t limit;
};

struct rte_aqm_red_stats {
	uint64_t delay;
	uint32_t length_bytes;
	uint16_t length_pkts;
};

struct rte_aqm_wred_params {
	uint16_t limit;
};

struct rte_aqm_wred_stats {
	uint64_t delay;
	uint32_t length_bytes;
	uint16_t length_pkts;
};

struct rte_aqm_pie_params {
	uint16_t limit;
};

struct rte_aqm_pie_stats {
	uint64_t delay;
	uint32_t length_bytes;
	uint16_t length_pkts;
};

struct rte_aqm_codel_params {
	uint16_t limit;
};

struct rte_aqm_codel_stats {
	uint64_t delay;
	uint32_t length_bytes;
	uint16_t length_pkts;
};

#ifdef __cplusplus
}
#endif

#endif /* _RTE_AQM_ALGORITHM_H_ */
