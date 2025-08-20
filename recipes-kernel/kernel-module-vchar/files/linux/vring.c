/*
 * SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries
 */

/*-------------------------------------------------------------------------*/

/*
 * Virtual ring implementation provides API to initialize, send and receive
 * data through other driver provided memory pool. It might be applicable
 * for local communications as for virtualized communications over shared
 * memory. It provides ability to implement as synchrnous as asynchronous
 * upper layer communications.
 *
 * It defines 2 memory sections , one for tx and one for rx (each
 * unidirectional, it means that tx is writable at side A and read-only at
 * side B, rx vice-versa is read-only at side A and writable at side B).
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME "[%s]:%u " fmt, __func__, __LINE__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <asm/barrier.h>
#include <asm/cacheflush.h>
#include <vring.h>

uint32_t virtual_ring_get_magic(void)
{
	union {
		uint64_t u64;
		uint32_t u32[2];
	} kt;
	uint32_t magic;

	/* generate new local side magic */
	kt.u64 = ktime_to_ns(ktime_get_real());
	/*
	 * small optimization, use u32 instead of u64 comparison every
	 * virtual_ring_check_ready
	 */
	magic = kt.u32[0] ^ kt.u32[1];

	return magic;
}

MODULE_AUTHOR("Qualcomm Technologies, Inc. ");
MODULE_DESCRIPTION("Qualcomm Technologies vring support");
MODULE_ALIAS("platform:vring");
MODULE_LICENSE("GPL");
