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
#ifdef __LINUX__
#include <linux/kernel.h>
#include <linux/module.h>
#include <coqoshv/coqoshv.h>
#endif
#include <vchar.h>
#include <vchar_private.h>

/* Default number of blocks in case none is provided */
#define DEFAULT_NUMBER_OF_BLOCKS  16

/*
 * vchar_setup_vring - allocated vring and setups tx info
 * Arguments:
 *	ctx:		pointer to vchar_context_s descriptor
 * Return value:
   -ENODEV - failed to identify VCHAR device.
   -EFAULT - failed to sync/resync with opposite side.
   0 - Success.
*/
static int vchar_setup_vring(struct vchar_context_s *ctx);

static int vchar_setup_vring(struct vchar_context_s *c)
{
	struct virtual_ring_s *vring;
	struct vchar_info *info = &c->info;

	/* validate number of blocks (if 0, set default value) */
	if (info->tx_msg_nr == 0)
		info->tx_msg_nr = DEFAULT_NUMBER_OF_BLOCKS;

	if (info->rx_msg_nr == 0)
		info->rx_msg_nr = DEFAULT_NUMBER_OF_BLOCKS;

	/*
	 * virtual ring setup with memory barriers and disabled cache
	 * flush & invalidation
	 */
	if (virtual_ring_setup(&vring, ((void *)(uintptr_t) info->tx_kvaddr), info->tx_len,
			info->tx_msg_nr, ((void *)(uintptr_t) info->rx_kvaddr),
			info->rx_len, info->rx_msg_nr, info->flags)) {
		pr_err("failed vring setup\n");
		vchar_cleanup_vring(c);
		return -ENODEV;
	}

	c->vring = vring;

	if (virtual_ring_resync(vring)) {
		pr_err("resync failed\n");
		vchar_cleanup_vring(c);
		return -EFAULT;
	}

	if (virtual_ring_sync(vring)) {
		pr_err("cannot sync\n");
		vchar_cleanup_vring(c);
		return -EFAULT;
	}

	/* setup static tx vchar information */
	c->info.tx_msg_sz = virtual_ring_get_tx_message_size(vring);
	/* c->vchar->info.tx_msg_sz is set in dev_open after oposit side ready */

	pr_info("allocated vring successfully\n");
	return 0;
}

int vchar_init(struct vchar_context_s *c)
{

	/* setup IPC */
	if (vchar_setup_mem(c)) {
		pr_err("critical vchar %s does not have assigned vring\n",
				c->name);
		return -ENODEV;
	}

	if (vchar_setup_vring(c)) {
		pr_err("unable to initialize vring.\n");
		return -ENODEV;
	}

	return 0;
}

void vchar_cleanup_vring(struct vchar_context_s *c)
{
	if (c) {
		struct virtual_ring_s *vring = c->vring;

		if (vring) {
			virtual_ring_cleanup(&vring);
			pr_info("vring cleanup done\n");
			c->vring = NULL;
		}

		vchar_cleanup_mem(c);
	}
}

int vchar_open(struct vchar_context_s *ctx)
{
	int res;

	if (!ctx) {
		pr_err("context is not set\n");
		return -EINVAL;
	}

	/* only one user of the channel at one time */
	if (atomic_cmpxchg(&ctx->access, 0, 1) != 0) {
		pr_err("channel is already opened\n");
		return -EBUSY;
	}

	/*
	 * this forces device to resetup connection during re-open,
	 * usually it is done at platform device setup
	 */
#ifdef RESYNC_ON_OPEN
	if ((res = virtual_ring_resync(ctx->vring))) {
		pr_err("resync failed\n");
		goto error;
	}
#endif

	if ((res = virtual_ring_sync(ctx->vring))) {
		pr_err("cannot sync\n");
		goto error;
	}

	if ((res = virtual_ring_wait_opposite_interruptible_timeout(ctx->vring,
			ctx->info.open_timeout))) {
		pr_err("cannot wait or wait was interrupted\n");
		goto error;
	}

	/* update setup specific vchar info data */
	res = virtual_ring_get_rx_message_size(ctx->vring);
	if (res < 0) {
		pr_err("rx message size get error\n");
		goto error;
	}
	ctx->info.rx_msg_sz = res;

	res = virtual_ring_validate_rx_messages_number(ctx->vring);
	if (res < 0) {
		pr_err("rx messages number get error\n");
		goto error;
	}

	return 0;

error:
	atomic_set(&ctx->access, 0);
	return res;
}

int vchar_close(struct vchar_context_s *ctx)
{
	if (!ctx) {
		pr_err("context is not set\n");
		return -EINVAL;
	}

	if (atomic_cmpxchg(&ctx->access, 1, 0) != 1) {
		pr_err("channel has been already released\n");
		return -EBUSY;
	}

	return 0;
}

ssize_t vchar_recv(struct vchar_context_s *ctx, uint8_t *buf, size_t len)
{
	int ret;
	int res;
	uint32_t l = len;

	if (!ctx) {
		pr_err("cannot get vchar_context_s from filep\n");
		return -EINVAL;
	}

	if (virtual_ring_sync(ctx->vring)) {
		pr_err("sync failed\n");
		return -EFAULT;
	}

	if (virtual_ring_check_ready(ctx->vring)) {
		ret = vchar_wait_interruptible(ctx);
		if (ret) {
			return ret;
		}
	}

	do {
		if (virtual_ring_sync(ctx->vring)) {
			pr_err("sync failed\n");
			return -EFAULT;
		}

		res = virtual_ring_recv_get_pending(ctx->vring);
		if (res > 0) {
			break;
		} else if (res == 0) {
			ret = vchar_wait_interruptible_timeout(ctx, VCHAR_WAIT_EVENT_TIMEOUT);
			if (ret) {
				return ret;
			}
		} else {
			/* error */
			pr_err("error\n");
			return -EFAULT;
		}
	} while (1);

	ret = virtual_ring_recv(ctx->vring, buf, &l);
	if (ret) {
		pr_err("recv failed\n");
		return ret;
	}

	if (l > len) {
		pr_err("provided buffer is too small, cutting the message\n");
		l = len;
	}

	return l;
}

ssize_t vchar_send(struct vchar_context_s *ctx, const uint8_t *buffer, size_t len)
{
	int ret;
	int res;
	uint8_t	*buf;

	if (!ctx) {
		pr_err("cannot get vchar_context_s from filep\n");
		return -EINVAL;
	}

	ret = virtual_ring_get_tx_message_size(ctx->vring);

	if (ret < 0)
		return ret;

	if (len > (size_t) ret) {
		pr_err("too big packet\n");
		return -EINVAL;
	}

	if (virtual_ring_sync(ctx->vring)) {
		pr_err("sync failed\n");
		return -EFAULT;
	}

	if (virtual_ring_check_ready(ctx->vring)) {
		ret = vchar_wait_interruptible(ctx);
		if (ret) {
			return ret;
		}
	}

	do {
		if (virtual_ring_sync(ctx->vring)) {
			pr_err("sync failed\n");
			return -EFAULT;
		}

		res = virtual_ring_send_get_buffer(ctx->vring, &buf);
		if (res == 0) {
			break;
		} else {
			ret = vchar_wake_up_interruptible_timeout(ctx, VCHAR_WAKE_UP_EVENT_TIMEOUT);
			if (ret) {
				return ret;
			}
		}
	} while (1);

	ret = ixcf_mem_copy(buf, buffer, len);
	if (ret) {
		virtual_ring_send_cancel(ctx->vring);
		return -EFAULT;
	}

	if (virtual_ring_send(ctx->vring, len)) {
		pr_err("cannot send buffer\n");
		return -EAGAIN;
	}

	/* ping the other side */
	vchar_notify_func(ctx);

	return len;
}
