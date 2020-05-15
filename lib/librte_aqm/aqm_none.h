/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _AQM_NONE_H_
#define _AQM_NONE_H_

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * No AQM
 */

#include <rte_mbuf.h>

#include "rte_aqm_algorithm.h"

struct aqm_none;

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
size_t aqm_none_get_memory_size(struct rte_aqm_none_params *params);

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
int aqm_none_init(struct aqm_none *none, struct rte_aqm_none_params *params);

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
int aqm_none_destroy(struct aqm_none *none);

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
int aqm_none_enqueue(struct aqm_none *none, struct rte_mbuf *pkt);

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
int aqm_none_dequeue(struct aqm_none *none, struct rte_mbuf **pkt);

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
int aqm_none_get_stats(struct aqm_none *none, struct rte_aqm_none_stats *stats);

#endif /* _AQM_NONE_H_ */
