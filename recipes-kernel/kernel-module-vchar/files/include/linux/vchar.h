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

#ifndef __COQOS_LINUX_VCHAR_H__
#define __COQOS_LINUX_VCHAR_H__

/* char device related defines */
#define DEFAULT_DEVICE_NAME "devvchar"
#define MINORS_PER_DEVICE (8)

/* enumeration of memory resources expected in device tree */
#define VCHAR_TX_MEMORY_RESOURCE 0
#define VCHAR_RX_MEMORY_RESOURCE 1
#define VCHAR_NAME_AND_MMAP_MEMORY_RESOURCE 2
#define VCHAR_IRQ_RESOURCE 0

/* Internal magic/offset to avoid device tree item values being interpreted as real values */
#define VCHAR_OF_MAGIC 0xdfecba00
#define VCHAR_NO_NOTIFY_SUPPORT (VCHAR_OF_MAGIC + 1)
#define VCHAR_NO_IRQ_SUPPORT (VCHAR_OF_MAGIC + 2)

struct vchar_context_s;

#ifdef __KERNEL__
/*
 * vchar_get_ctx - look up for a context by name.
 * Arguments:
 *	name:		channel's character name.
 * Return value:
   NULL - channel was not found.
   non-NULL pointer indicates successfully found context structure.
*/
struct vchar_context_s * vchar_get_ctx(const char *name);

/*
 * vchar_poll - wait for some event on a VCHAR's channel.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 *	req_events:	requested events (POLLIN/POLLRDNORM/POLLOUT/POLLWRNORM)
 *	timeout:	timeout in jiffies
 * Return value:
 * -EFAULT - failed to sync/resync with opposite side.
 * -EINVAL - invalid arguments (ctx or file pointer associated with the channel
 *           is not set)
 * 0 - indicates that the call timed out and the channel was not ready.
 *     non-negative value - indicates a mask with events requested.
*/
int vchar_poll(struct vchar_context_s *ctx, unsigned long req_events, long timeout);
#endif

#endif /* __COQOS_LINUX_VCHAR_H__ */
