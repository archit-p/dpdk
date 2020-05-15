/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#ifndef _PKT_DESC_RING_H_
#define _PKT_DESC_RING_H_

/**
 * @internal
 *
 * @warning
 * @b EXPERIMENTAL: All APIs in this file may change without prior notice.
 *
 * @file
 * Packet Descriptor Ring for AQM
 */

#include <stdint.h>

#include <rte_mbuf.h>

struct pkt_desc_ring;

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
size_t pkt_desc_ring_get_memory_size(uint16_t limit);

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
int pkt_desc_ring_init(struct pkt_desc_ring *pdr, uint16_t limit);

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
int pkt_desc_ring_destroy(struct pkt_desc_ring *pdr);

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
uint8_t pkt_desc_ring_is_empty(struct pkt_desc_ring *pdr);

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
uint8_t pkt_desc_ring_is_full(struct pkt_desc_ring *pdr);

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
int pkt_desc_ring_enqueue(struct pkt_desc_ring *pdr, struct rte_mbuf *pkt);

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
int pkt_desc_ring_dequeue(struct pkt_desc_ring *pdr, struct rte_mbuf **pkt);

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
uint16_t pkt_desc_ring_get_length_pkts(struct pkt_desc_ring *pdr);

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
uint32_t pkt_desc_ring_get_length_bytes(struct pkt_desc_ring *pdr);

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
uint64_t pkt_desc_ring_get_delay(struct pkt_desc_ring *pdr);

#endif /* _PKT_DESC_RING_H_ */
