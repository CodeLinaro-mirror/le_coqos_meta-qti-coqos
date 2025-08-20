/*
 * SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries
 */

/*-------------------------------------------------------------------------*/

/*
 * The file is a platform abstraction header that contains
 * platform compatible function definitions to work with cache.
 */

#ifndef __COQOS_LINUX_IXCF_CACHE_H__
#define __COQOS_LINUX_IXCF_CACHE_H__

/*
 * Invalidate data cache by address to Point of Coherency
 * for given range [ addr + len ].
 *
 * Parameters:
 *  addr - kernel address
 *  len - size of region
*/
static inline void ixcf_invalidate_memory_region(void *addr, size_t len)
{
	uint64_t x2;
	uint64_t x3;

	asm volatile("mrs %[x3], ctr_el0\n"
	     "ubfm %[x3], %[x3], #16, #19\n"
	     "mov %[x2], #0x4\n"
	     "lsl %[x2], %[x2], %[x3]\n"
	     "sub %[x3], %[x2], #0x1\n"
	     "add %[len], %[addr], %[len]\n"
	     "bic %[addr], %[addr], %[x3]\n"
	     /* IVAC instruction causes Permission Fault exception
	      * because it requires write access permission to the VA
	      * but the driver calls this function passing pointer on
	      * read-only memory.
	     */
	     "1: dc civac, %[addr]\n"
	     "add %[addr], %[addr], %[x2]\n"
	     "cmp %[addr], %[len]\n"
	     "b.lo 1b\n"
	     "dsb sy\n"
	     : [x2] "=&r" (x2), [x3] "=&r" (x3), [addr] "+r" (addr), [len] "+r" (len)
             :
	     : "memory");
}

/*
 * Clean and Invalidate data cache by address to Point of Coherency
 * for given range [ addr + len ].
 *
 * Parameters:
 *  addr - kernel address
 *  len - size of region
*/
static inline void ixcf_flush_memory_region(void *addr, size_t len)
{
	uint64_t x2;
	uint64_t x3;

	asm volatile("mrs %[x3], ctr_el0\n"
	     "ubfm %[x3], %[x3], #16, #19\n"
	     "mov %[x2], #0x4\n"
	     "lsl %[x2], %[x2], %[x3]\n"
	     "sub %[x3], %[x2], #0x1\n"
	     "add %[len], %[addr], %[len]\n"
	     "bic %[addr], %[addr], %[x3]\n"
	     "1: dc civac, %[addr]\n"
	     "add %[addr], %[addr], %[x2]\n"
	     "cmp %[addr], %[len]\n"
	     "b.lo 1b\n"
	     "dsb sy\n"
	     : [x2] "=&r" (x2), [x3] "=&r" (x3), [addr] "+r" (addr), [len] "+r" (len)
             :
	     : "memory");
}

#endif /* __COQOS_LINUX_IXCF_CACHE_H__ */
