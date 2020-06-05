# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2020 NITK Surathkal

sources = files('rte_aqm.c',
		'pkt_desc_ring.c',
		'aqm_none.c',
		'aqm_red.c',
		'aqm_wred.c',
		'aqm_pie.c',
		'aqm_codel.c')

headers = files('rte_aqm.h',
		'rte_aqm_algorithm.h',
		'rte_red.h')

deps += ['eal', 'mbuf', 'ring']
