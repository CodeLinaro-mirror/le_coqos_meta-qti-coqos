/*
 * SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries
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
#ifndef COQOS_VCHAR_PRIVATE_H
#define COQOS_VCHAR_PRIVATE_H

#define MAX_DEVICE_NAME (32)

struct vchar_context_s {
	char				name[MAX_DEVICE_NAME];
	struct virtual_ring_s		*vring;
	struct virtual_char_s		*vchar;
	struct vchar_info		info;
	atomic_t			access;
	uint32_t			flags; /* set of supported features */
};

/*
 * vchar_init - does necessary init sequence.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
			the memory shall be allocated by user.
 * Return value:
   -EFAULT - failed to sync/resync with opposite side.
   -EINVAL - invalid arguments
   0 - Success.
*/
int vchar_init(struct vchar_context_s *ctx);

/*
 * vchar_cleanup_vring - does necessary clean up sequence
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 * Return value:
   None.
*/
void vchar_cleanup_vring(struct vchar_context_s *ctx);

/*******************************************************************************
 * Callback functions
 ******************************************************************************/
int vchar_wait_interruptible(struct vchar_context_s *ctx);
/* Parameter timeout in vchar_wait_interruptible_timeout() */
#define VCHAR_WAIT_EVENT_TIMEOUT 100
int vchar_wait_interruptible_timeout(struct vchar_context_s *ctx, long timeout);
/* Parameter timeout in vchar_wake_up_interruptible_timeout() */
#define VCHAR_WAKE_UP_EVENT_TIMEOUT 1
int vchar_wake_up_interruptible_timeout(struct vchar_context_s *ctx, long timeout);
void vchar_notify_func(struct vchar_context_s *ctx);
int vchar_setup_mem(struct vchar_context_s *c);
void vchar_cleanup_mem(struct vchar_context_s *c);

#endif /* #ifndef COQOS_VCHAR_PRIVATE_H */
