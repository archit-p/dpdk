/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _AQM_CODEL_H_
#define _AQM_CODEL_H_

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * Controlled Delay (CoDel) AQM
 */

#include <rte_mbuf.h>

#include "rte_aqm_algorithm.h"

struct aqm_codel;

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
size_t aqm_codel_get_memory_size(struct rte_aqm_codel_params *params);

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
int aqm_codel_init(struct aqm_codel *codel,
		   struct rte_aqm_codel_params *params);

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
int aqm_codel_destroy(struct aqm_codel *codel);

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
int aqm_codel_enqueue(struct aqm_codel *codel, struct rte_mbuf *pkt);

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
int aqm_codel_dequeue(struct aqm_codel *codel, struct rte_mbuf **pkt);

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
int aqm_codel_get_stats(struct aqm_codel *codel,
			struct rte_aqm_codel_stats *stats);

#endif /* _AQM_CODEL_H_ */
