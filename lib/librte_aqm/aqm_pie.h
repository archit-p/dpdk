/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _AQM_PIE_H_
#define _AQM_PIE_H_

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * Proportional Integral controller Enhanced (PIE) AQM
 */

#include <rte_mbuf.h>

#include "rte_aqm_algorithm.h"

struct aqm_pie;

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
size_t aqm_pie_get_memory_size(struct rte_aqm_pie_params *params);

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
int aqm_pie_init(struct aqm_pie *pie, struct rte_aqm_pie_params *params);

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
int aqm_pie_destroy(struct aqm_pie *pie);

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
int aqm_pie_enqueue(struct aqm_pie *pie, struct rte_mbuf *pkt);

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
int aqm_pie_dequeue(struct aqm_pie *pie, struct rte_mbuf **pkt);

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: This API may change without prior notice.
 *
 * @param
 *
 * @return
 */
int aqm_pie_get_stats(struct aqm_pie *pie, struct rte_aqm_pie_stats *stats);

#endif /* _AQM_PIE_H_ */
