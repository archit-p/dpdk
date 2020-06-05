/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _AQM_WRED_H_
#define _AQM_WRED_H_

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * Weighted Random Early Detection (WRED) AQM
 */

#include <rte_mbuf.h>

#include "rte_aqm_algorithm.h"

struct aqm_wred;

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
size_t aqm_wred_get_memory_size(struct rte_aqm_wred_params *params);

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
int aqm_wred_init(struct aqm_wred *wred, struct rte_aqm_wred_params *params);

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
int aqm_wred_destroy(struct aqm_wred *wred);

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
int aqm_wred_enqueue(struct aqm_wred *wred, struct rte_mbuf *pkt);

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
int aqm_wred_dequeue(struct aqm_wred *wred, struct rte_mbuf **pkt);

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
int aqm_wred_get_stats(struct aqm_wred *wred, struct rte_aqm_wred_stats *stats);

#endif /* _AQM_WRED_H_ */
