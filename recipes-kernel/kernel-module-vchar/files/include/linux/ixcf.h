/*
 * SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries
 */

/*-------------------------------------------------------------------------*/

/*
 * The file is a platform abstraction header that contains
 * platform compatible function definitions to work with memory, atomics.
 */

#ifndef __COQOS_LINUX_IXCF_H__
#define __COQOS_LINUX_IXCF_H__

#include <linux/io.h>
#include <linux/atomic.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <ixcf-cache.h>

/* Redefinitions for memory related functions */

/* ixcf_mem_copy - copies n bytes from memory area src to memory area dest.
 * NOTE: This function works only with kernel pointers.
 * Arguments:
 *	dest:		pointer to destination area
 *	src:		pointer to source area
 *	n:		number of bytes to be copied
 * Return value:
   number of bytes that could not be copied - on error
   0 - Success.
*/
static inline int ixcf_mem_copy(void *dest, const void *src, size_t n)
{
	void *p;

	if (!dest || !src) {
		return n;
	}

	if (!n) {
		return n;
	}

	p = memcpy(dest, src, n);
	if (p != dest) {
		return n;
	}

	return 0;
}

#define ixcf_alloc(size) kzalloc(size, GFP_KERNEL)
#define ixcf_free(ptr) kfree(ptr)
#define ixcf_memset memset
#define ixcf_read_once(a) READ_ONCE(a)

/* Redefinition for atomics functions */
#define ixcf_atomic_set atomic_set
#define ixcf_atomic_inc atomic_inc
#define ixcf_atomic_read atomic_read
#define ixcf_atomic_cmpxchg atomic_cmpxchg

#define ixcf_sleep schedule_timeout_interruptible

#endif /* __COQOS_LINUX_IXCF_H__ */
