/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _AQM_RED_H_
#define _AQM_RED_H_

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * Random Early Detection (RED) AQM
 */

#include <rte_mbuf.h>

#include "rte_aqm_algorithm.h"

struct aqm_red;

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
size_t aqm_red_get_memory_size(struct rte_aqm_red_params *params);

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
int aqm_red_init(struct aqm_red *red, struct rte_aqm_red_params *params);

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
int aqm_red_destroy(struct aqm_red *red);

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
int aqm_red_enqueue(struct aqm_red *red, struct rte_mbuf *pkt);

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
int aqm_red_dequeue(struct aqm_red *red, struct rte_mbuf **pkt);

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
int aqm_red_get_stats(struct aqm_red *red, struct rte_aqm_red_stats *stats);

#endif /* _AQM_RED_H_ */
