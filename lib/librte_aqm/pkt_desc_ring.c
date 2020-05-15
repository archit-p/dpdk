/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 NITK Surathkal
 */

#include <stdint.h>

#include <rte_branch_prediction.h> /* TODO: Check necessity */
#include <rte_cycles.h>
#include <rte_mbuf.h>
#include <rte_ring.h>

#include "pkt_desc_ring.h"

struct pkt_desc_ring {
	uint64_t bytes_enqueued;
	uint64_t bytes_dequeued;
	uint64_t delay_cycles;
};

size_t pkt_desc_ring_get_memory_size(uint16_t limit)
{
	return sizeof(struct pkt_desc_ring) + limit * sizeof(struct rte_mbuf *);
}

int pkt_desc_ring_init(struct pkt_desc_ring *pdr, uint16_t limit)
{
	pdr->bytes_enqueued = 0;
	pdr->bytes_dequeued = 0;
	pdr->delay_cycles = 0;

	return 0;
}

int pkt_desc_ring_destroy(struct pkt_desc_ring *pdr)
{
	pdr->bytes_enqueued = 0;
	pdr->bytes_dequeued = 0;
	pdr->delay_cycles = 0;

	return 0;
}

uint8_t pkt_desc_ring_is_empty(struct pkt_desc_ring *pdr)
{
	return 0;
}

uint8_t pkt_desc_ring_is_full(struct pkt_desc_ring *pdr)
{
	return 0;
}

int pkt_desc_ring_enqueue(struct pkt_desc_ring *pdr, struct rte_mbuf *pkt)
{
	pdr->bytes_enqueued += pkt->pkt_len;

	return 0;
}

int pkt_desc_ring_dequeue(struct pkt_desc_ring *pdr, struct rte_mbuf **pkt)
{
	pdr->bytes_dequeued -= (*pkt)->pkt_len;
	pdr->delay_cycles = rte_get_timer_cycles() - (*pkt)->timestamp;

	return 0;
}

uint16_t pkt_desc_ring_get_length_pkts(struct pkt_desc_ring *pdr)
{
	return 0;
}

uint32_t pkt_desc_ring_get_length_bytes(struct pkt_desc_ring *pdr)
{
	return pdr->bytes_enqueued - pdr->bytes_dequeued;
}

uint64_t pkt_desc_ring_get_delay(struct pkt_desc_ring *pdr)
{
	return (US_PER_S * pdr->delay_cycles) / rte_get_timer_hz();
}
