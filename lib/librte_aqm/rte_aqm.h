/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _RTE_AQM_H_
#define _RTE_AQM_H_

/**
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * RTE Active Queue Management (AQM)
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <rte_mbuf.h>

#include "rte_aqm_algorithm.h"

struct rte_aqm;

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
__rte_experimental
size_t rte_aqm_get_memory_size(void *params, enum rte_aqm_algorithm algorithm);

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
__rte_experimental
int rte_aqm_init(struct rte_aqm *aqm, void *params,
		 enum rte_aqm_algorithm algorithm);

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
__rte_experimental
int rte_aqm_destroy(struct rte_aqm *aqm);

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
__rte_experimental
int rte_aqm_enqueue(struct rte_aqm *aqm, struct rte_mbuf *pkt);

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
__rte_experimental
int rte_aqm_dequeue(struct rte_aqm *aqm, struct rte_mbuf **pkt);

/**
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
__rte_experimental
int rte_aqm_get_stats(struct rte_aqm *aqm, void *stats);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_AQM_H_ */
