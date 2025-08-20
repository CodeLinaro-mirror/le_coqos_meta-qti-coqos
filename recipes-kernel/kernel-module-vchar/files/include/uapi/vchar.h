/*
 * SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*-------------------------------------------------------------------------*/

/*
 * Virtual character device implements user space character device file
 * provider with support of synchronous and asynchronous IO,
 * bi-directional pipe logic and fixed maximum message size. It also
 * provides ability to have mmaped memory blocks for big or shared data.
 *
 * The virtual char device implements an interface for fast and simple
 * asynchronous inter-VM communications under the CoqosHv hypervisor.
 *
 */

#ifndef __VIRTUAL_CHAR_H__
#define __VIRTUAL_CHAR_H__

#ifdef __LINUX__
#include <linux/types.h>
#endif

/* virtual char information IOCTL structure */
struct vchar_info {
	/* shmem related info */
	uint64_t shmem_paddr;
	uint32_t shmem_len;

	/* irq and notification related info */
	uint32_t irq_nr;
	uint32_t irqcap_nr;

	/* supported features from DTS config */
	uint32_t flags;

	/* tx pipe related info */
	uint64_t tx_paddr;
	uint64_t tx_kvaddr;
	uint32_t tx_len;
	uint32_t tx_msg_sz;
	uint32_t tx_msg_nr;

	/* rx pipe related info */
	uint64_t rx_paddr;
	uint64_t rx_kvaddr;
	uint32_t rx_len;
	uint32_t rx_msg_sz;
	uint32_t rx_msg_nr;

	/* rx irq events tracking info */
	uint64_t rx_int_ctr;

	/* timeout for vchar_open to wait opposite side at sync time */
	long open_timeout;
};

#ifdef __LINUX__
/* virtual char device ioctl magic number */
#define VCHAR_IOCTL_MAGIC 0xFE

/* virtual char device get info ioctl id */
#define	VCHAR_INFO _IOR(VCHAR_IOCTL_MAGIC, 0x01, struct vchar_info)
#endif

#endif /* ifndef __VIRTUAL_CHAR_H__ */
