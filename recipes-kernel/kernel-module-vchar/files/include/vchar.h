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

#ifndef __COQOS_VIRTUAL_CHAR_H__
#define __COQOS_VIRTUAL_CHAR_H__

#include <vring.h>
#include <uapi/vchar.h>
/* Cross platform functions are defined in platform related header file. */
#ifdef __LINUX__
#include "linux/vchar.h"
#endif

/* High 2 bytes are reserved for VCHAR flags */

/* Enforces virtual char to not allocate device's node. */
#define VIRTUAL_CHAR_DISABLE_NODE	(0x1 << 16)
#define VIRTUAL_CHAR_NONBLOCK_MODE	(0x2 << 16)

struct vchar_context_s;

/*
 * vchar_open - does necessary sequence on open.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 * Return value:
   -EFAULT - failed to sync/resync with opposite side.
   -ERESTARTSYS - failed to wait for data.
   -EINVAL - invalid arguments
   -EBUSY - channel is already opened
   0 - Success.
*/
int vchar_open(struct vchar_context_s *ctx);

/*
 * vchar_close - release the channel.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 * Return value:
 * -EINVAL - invalid arguments
 * -EBUSY - channel is already closed
 * 0 - Success.
*/
int vchar_close(struct vchar_context_s *ctx);

/*
 * vchar_recv - receives data.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 *	uint8_t *buf: pointer to buffer to copy data in.
 *	size_t len:	data length.
 * Return value:
   -EFAULT - failed to sync/resync with opposite side.
   -EINVAL - invalid arguments
   -EAGAIN - failed to receive data.
   On success, the function returns the number of bytes received.
*/
ssize_t vchar_recv(struct vchar_context_s *ctx, uint8_t *buf, size_t len);

/*
 * vchar_send - sends data.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 *	const uint8_t *buf: pointer to buffer to send the data out.
 *	size_t len:	data length.
 * Return value:
   -EFAULT - failed to sync/resync with opposite side.
   -EINVAL - invalid arguments
   -EAGAIN - failed to send data.
   On success, the function returns the number of bytes sent.
*/
ssize_t vchar_send(struct vchar_context_s *ctx, const uint8_t *buf, size_t len);

#if !(defined(__LINUX__) && !defined(__KERNEL__))
/*
 * vchar_mmap - sends data.
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 *	void **addr:	double pointer to mapped memory address to save into.
 *	size_t *size:	pointer to returned memory size.
 * Return value:
   -EINVAL - if the ctx/addr/size pointers are not set.
   -ENODEV - if the shared memory region data is not available in the context.
   -ENOMEM - if mmap operation failed.
   -ENOTSUP - if mmap operation is not supported.
   0 - Success.
*/
int vchar_mmap(struct vchar_context_s *ctx, void **addr, size_t *size);
#endif

#endif /* ifndef __COQOS_VIRTUAL_CHAR_H__ */
