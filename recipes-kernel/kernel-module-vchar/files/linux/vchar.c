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

#define pr_fmt(fmt) KBUILD_MODNAME "[%s]:%u " fmt, __func__, __LINE__
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/list.h>
#include <linux/version.h>
#if KERNEL_VERSION(5, 14, 0) <= LINUX_VERSION_CODE
#include <linux/panic_notifier.h>
#endif
#include <vchar.h>
#include <vchar_private.h>
#include <coqoshv/coqoshv.h>

/* char device related context structure */
struct char_dev_node_s {
	struct class			*class;
	char				class_name[MAX_DEVICE_NAME];
	struct device			*device;
	dev_t				major;
	struct cdev			cdev;
	bool				cdev_initialized;
};

struct char_dev_context_s {
	char				dev_name[MAX_DEVICE_NAME];
	struct char_dev_node_s		*node;
	wait_queue_head_t		rx_wq;
	wait_queue_head_t		tx_wq;
	struct task_struct		*thread;
	wait_queue_head_t		thread_wq;
	uint32_t			irq;
	uint32_t			irqcap; /* opposite side notification */
};

/* mapped memory related context structure */
struct mem_region_s {
	phys_addr_t			mem;
	size_t				len;
};

struct virtual_char_s
{
	struct vchar_context_s		*ctx;
	struct mem_region_s		*mem;
	struct char_dev_context_s	*dev;
	struct platform_device		*pdev;
	/* robustness block for reboot */
	struct notifier_block		reboot_notifier;
	/* robustness block for panic or other events */
	struct notifier_block		panic_notifier;
	struct list_head		list;
};

/* Convert filep->f_flags into ctx->flags*/
static uint32_t f_flags_to_vchar_flags(unsigned int f_flags)
{
	return f_flags & O_NONBLOCK ? VIRTUAL_CHAR_NONBLOCK_MODE : 0;
}

static int vchar_notifier_handler(struct virtual_ring_s *p)
{
	pr_info("notification gathered, trying to tell oposit we are dead\n");
	return NOTIFY_DONE;
}

/*
 * we have emergency case notifier to let the other side of ring to know that
 * we are rebooting
 */
static int reboot_notifier_handler(struct notifier_block *n,
		unsigned long val, void *arg)
{
	struct virtual_char_s *c = container_of(n, struct virtual_char_s,
			reboot_notifier);
	return vchar_notifier_handler(c->ctx->vring);
}

/*
 * we have emergency case notifier to let the other side of ring to know that
 * we are dead
 */
static int panic_notifier_handler(struct notifier_block *n,
		unsigned long val, void *arg)
{
	struct virtual_char_s *c = container_of(n, struct virtual_char_s,
			panic_notifier);
	return vchar_notifier_handler(c->ctx->vring);
}

static void vchar_clean_reboot_notifiers(struct vchar_context_s *c)
{
	struct virtual_char_s *p = c->vchar;

	unregister_reboot_notifier(&p->reboot_notifier);
	atomic_notifier_chain_unregister(&panic_notifier_list,
			&p->panic_notifier);
}

static void vchar_setup_reboot_notifiers(struct vchar_context_s *c)
{
	struct virtual_char_s *p = c->vchar;

	/* subscribe for panic and reboot events */
	p->reboot_notifier.notifier_call = reboot_notifier_handler;
	register_reboot_notifier(&p->reboot_notifier);

	p->panic_notifier.notifier_call = panic_notifier_handler;
	atomic_notifier_chain_register(&panic_notifier_list,
		&p->panic_notifier);
}

/* Hypercall invoke for a particular id exported by coqoshv */
#define coqoshv_hypercall_irqcap(gate) \
	p_syscall1(HVC_P_TRIGGER_CAP, gate)

void vchar_notify_func(struct vchar_context_s *ctx)
{
	struct char_dev_context_s *dev = ctx->vchar->dev;

	if (!ctx || !dev) {
		pr_err("no context\n");
		return;
	}

	if (dev->irqcap != VCHAR_NO_NOTIFY_SUPPORT) {
		coqoshv_hypercall_irqcap(dev->irqcap);
	}
}

LIST_HEAD(vchar_channels);

struct vchar_context_s * vchar_get_ctx(const char *name)
{
	struct virtual_char_s *p;

	list_for_each_entry(p, &vchar_channels, list) {
		if (p && p->ctx && !strcmp(p->ctx->name, name)) {
			return p->ctx;
		}
	}

	return NULL;
}

EXPORT_SYMBOL(vchar_get_ctx);

/* char device file related functions */

static int dev_open(struct inode *inodep, struct file *filep)
{
	struct char_dev_node_s *node = container_of(inodep->i_cdev,
			struct char_dev_node_s, cdev);
	struct vchar_context_s *ctx;
	int ret;

	if (!node) {
		pr_err("cannot get char_dev_node_s from inode\n");
		return -EINVAL;
	}

	/* now get vchar ctx */
	ctx = (struct vchar_context_s *) dev_get_platdata(node->device);
	if (!ctx) {
		pr_err("cannot get vchar_context_s from driver data\n");
		return -EINVAL;
	}

	/*
	 * Ignore blocking/nonblocking mode set in device tree settings. Use
	 * file mode specified in open/read/write syscall.
	 */
	ctx->flags = f_flags_to_vchar_flags(filep->f_flags);
	ret = vchar_open(ctx);
	if (ret) {
		return ret;
	}

	/* now set filep->private_data field */
	filep->private_data = ctx;
	pr_info("open success\n");
	return 0;
}

EXPORT_SYMBOL(vchar_open);

static ssize_t dev_read(struct file *filep, char *buffer, size_t len,
		loff_t *offset)
{
	struct vchar_context_s *ctx =
		(struct vchar_context_s *) filep->private_data;
	uint8_t		*rcv_buf;
	ssize_t		bytes_received;

	if (!len) {
		return 0;
	}

	rcv_buf = kmalloc(len, GFP_KERNEL);
	if (!rcv_buf) {
		return -ENOMEM;
	}


	ctx->flags = f_flags_to_vchar_flags(filep->f_flags);

	bytes_received = vchar_recv(ctx, rcv_buf, len);
	if (bytes_received < 0) {
		kfree(rcv_buf);
		return bytes_received;
	} else if (bytes_received > len) {
		pr_err("vchar_recv() returned more bytes than requested (recv=%zd, req=%zu)!\n", bytes_received, len);
		bytes_received = len;
	}

	if (copy_to_user(buffer, rcv_buf, bytes_received)) {
		pr_err("copy to user failed\n");
		kfree(rcv_buf);
		return -EINVAL;
	}
	kfree(rcv_buf);

	return bytes_received;
}

EXPORT_SYMBOL(vchar_recv);

static ssize_t dev_write(struct file *filep, const char *buffer,
		size_t len, loff_t *offset)
{
	struct vchar_context_s *ctx =
		(struct vchar_context_s *) filep->private_data;
	uint8_t		*send_buf;
	ssize_t		bytes_sent;

	if (!len) {
		return 0;
	}

	send_buf = kmalloc(len, GFP_KERNEL);
	if (!send_buf) {
		return -ENOMEM;
	}

	if (copy_from_user(send_buf, buffer, len)) {
		pr_err("copy from user failed\n");
		kfree(send_buf);
		return -EINVAL;
	}

	ctx->flags = f_flags_to_vchar_flags(filep->f_flags);
	bytes_sent = vchar_send(ctx, send_buf, len);

	kfree(send_buf);

	return bytes_sent;
}

EXPORT_SYMBOL(vchar_send);

int vchar_wait_interruptible(struct vchar_context_s *ctx)
{
	if (ctx->flags & VIRTUAL_CHAR_NONBLOCK_MODE) {
#ifdef RESYNC_ON_OPEN
		pr_err("async vring not ready to work, reopen dev\n");
#endif
		return -EIO;
	} else {
		pr_warn("sync vring not ready to work, resynch\n");
		if (virtual_ring_wait_opposite_interruptible_timeout(ctx->vring,
				VRING_INFINITE_TIMEOUT)) {
			pr_err("interrupted\n");
			return -ERESTARTSYS;
		}
	}

	return 0;
}

int vchar_wait_interruptible_timeout(struct vchar_context_s *ctx, long timeout)
{
	if (ctx->flags & VIRTUAL_CHAR_NONBLOCK_MODE) {
		return -EAGAIN;
	} else {
		if (wait_event_interruptible_timeout(ctx->vchar->dev->rx_wq,
		    virtual_ring_recv_get_pending(ctx->vring) > 0, timeout) < 0) {
			pr_err("interrupted\n");
			return -EINTR;
		}
	}

	return 0;
}

int vchar_wake_up_interruptible_timeout(struct vchar_context_s *ctx, long timeout)
{
	if (ctx->flags & VIRTUAL_CHAR_NONBLOCK_MODE) {
		wake_up(&ctx->vchar->dev->thread_wq);
		return -EAGAIN;
	} else {
		if (schedule_timeout_interruptible(timeout)) {
			pr_err("interrupted\n");
			return -EINTR;
		}
	}

	return 0;
}

static unsigned int dev_poll(struct file *filep,
					struct poll_table_struct *poll_table)
{
	struct vchar_context_s *ctx = filep->private_data;
	unsigned int mask = 0;

	BUG_ON(!ctx);

	if (virtual_ring_sync(ctx->vring)) {
		pr_err("sync failed\n");
		return POLLHUP;
	}

	if (virtual_ring_check_ready(ctx->vring)) {
		/* vchar peer was restarted, user should re-open the device */
		return POLLHUP;
	}

	if (virtual_ring_recv_get_pending(ctx->vring) > 0)
		mask |= POLLIN | POLLRDNORM;

	poll_wait(filep, &ctx->vchar->dev->rx_wq, poll_table);

	/* Because the polling thread wakes up TX wait queue every 10 jiffies, avoid
	   adding TX work queue to a set of monitored queues, unless explicitly
	   asked by the user */
	if (poll_requested_events(poll_table) & (POLLOUT | POLLWRNORM)) {
		if (virtual_ring_send_get_avail(ctx->vring) > 0)
			mask |= POLLOUT | POLLWRNORM;

		poll_wait(filep, &ctx->vchar->dev->tx_wq, poll_table);
	}

	return mask;
}

int vchar_poll(struct vchar_context_s *ctx, unsigned long req_events, long timeout)
{
	unsigned int mask = 0;
	int ret;

	if (!ctx) {
		pr_err("vchar_context_s is not set\n");
		return -EINVAL;
	}

	if (virtual_ring_sync(ctx->vring)) {
		pr_err("sync failed\n");
		return -EFAULT;
	}

	if (virtual_ring_check_ready(ctx->vring)) {
		if (ctx->flags & VIRTUAL_CHAR_NONBLOCK_MODE) {
			pr_err("async vring not ready to work, reopen dev\n");
			return 0;
		} else {
			pr_warn("sync vring not ready to work, resync\n");
			ret = virtual_ring_wait_opposite_interruptible_timeout(ctx->vring, timeout);
			if (ret) {
				if (ret < 0) {
					pr_err("interrupted\n");
					return ret;
				}
				/* timeout */
				return 0;
			}
		}
	}

	/* check if it is POLLIN */
	if (req_events & POLLIN) {
		/* poll for the receiver side */
		if (virtual_ring_recv_get_pending(ctx->vring) == 0) {
			if (timeout) {
				if (!wait_event_interruptible_timeout(ctx->vchar->dev->rx_wq,
                                        virtual_ring_recv_get_pending(ctx->vring) > 0, timeout)) {
					/* timeout */
					return 0;
				}
			} else {
				wait_event_interruptible(
					ctx->vchar->dev->rx_wq,
					virtual_ring_recv_get_pending(ctx->vring) > 0);
			}
		}

		if (virtual_ring_recv_get_pending(ctx->vring) > 0)
			mask |= POLLIN | POLLRDNORM;
	}

	/* check if it is POLLOUT */
	if (req_events & POLLOUT) {
		/* check if POLLIN has not been triggered and we need wait */
		if (!mask) {
			if (virtual_ring_send_get_avail(ctx->vring) == 0) {
				wake_up(&ctx->vchar->dev->thread_wq);
				if (timeout) {
					if (!wait_event_interruptible_timeout(
						ctx->vchar->dev->tx_wq,
						virtual_ring_send_get_avail(
							ctx->vring) > 0, timeout)) {
						/* timeout */
						return 0;
					}
				} else {
					wait_event_interruptible(
						ctx->vchar->dev->tx_wq,
						virtual_ring_send_get_avail(
							ctx->vring) > 0);
				}
			}
		}
		if (virtual_ring_send_get_avail(ctx->vring) > 0)
			mask |= POLLOUT | POLLWRNORM;
	}

	return mask;
}

EXPORT_SYMBOL(vchar_poll);

static int dev_mmap(struct file *filep, struct vm_area_struct *vma)
{
	struct vchar_context_s *ctx =
		(struct vchar_context_s *) filep->private_data;
	phys_addr_t addr;
	unsigned long sz;

	if (!ctx) {
		pr_err("cannot get vchar_context_s from filep\n");
		return -EINVAL;
	}

	if (!ctx->vchar->mem) {
		pr_err("vring not ready to work, reopen the device\n");
		return -ENOSYS;
	}

	addr = ctx->vchar->mem->mem;
	sz = ctx->vchar->mem->len;

	pr_info("mapping %08zx:%lu\n", (size_t) addr, sz);

	/* do check PAGE align */
	if ((addr & (PAGE_SIZE - 1)) |
				(sz & (PAGE_SIZE - 1))) {
		pr_err("addr or sz are not page aligned error\n");
		return -ENOSYS;
	}

	/* set policy for the memory region to: normal uncached + write buffering */
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	/* do mapping to the user space */
	return vm_iomap_memory(vma, addr, sz);
}

int vchar_mmap(struct vchar_context_s *ctx, void **addr, size_t *size)
{
	if (!ctx) {
		pr_err("vchar: Pointer to context is not set.\n");
		return -EINVAL;
	}

	if (!addr || !size) {
		pr_err("vchar: Input address or size pointers are not set.\n");
		return -EINVAL;
	}

	if (!ctx->vchar) {
		pr_err("vchar: Config structure is not set in the context.\n");
		return -ENODEV;
	}

	if (!ctx->vchar->mem) {
		pr_err("vchar: Shared memory info structure is not set in the context.\n");
		return -ENODEV;
	}

	*addr = ioremap_cache(ctx->vchar->mem->mem, ctx->vchar->mem->len);
	if (!*addr) {
		pr_err("vchar: ioremap_cache() failed for address %llx size %lx.\n",
				ctx->vchar->mem->mem, ctx->vchar->mem->len);
		return -ENOMEM;
	}

	*size = ctx->vchar->mem->len;

	return 0;
}
EXPORT_SYMBOL(vchar_mmap);

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct vchar_context_s *ctx =
		(struct vchar_context_s *) filep->private_data;

	if (!ctx) {
		pr_err("cannot get vchar_context_s from filep\n");
		return -EINVAL;
	}

	ctx->flags = f_flags_to_vchar_flags(filep->f_flags);
	if (virtual_ring_sync(ctx->vring)) {
		pr_err("sync failed\n");
		return -EFAULT;
	}

	if (virtual_ring_check_ready(ctx->vring)) {
		if (ctx->flags & VIRTUAL_CHAR_NONBLOCK_MODE) {
			pr_err("async vring not ready to work, reopen dev\n");
			return -EFAULT;
		} else {
			pr_warn("sync vring not ready to work, resync\n");
			if (virtual_ring_wait_opposite_interruptible_timeout(
						ctx->vring, VRING_INFINITE_TIMEOUT)) {
				pr_err("interrupted\n");
				return -ERESTARTSYS;
			}
		}
	}

	if (cmd == VCHAR_INFO) {
		if (copy_to_user((void __user *)arg,
					&ctx->info, sizeof(ctx->info)))
			return -EFAULT;
	} else {
		pr_err("unsupported ioctl %x\n", cmd);
		return -EINVAL;
	}

	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	struct char_dev_node_s *node = container_of(inodep->i_cdev,
			struct char_dev_node_s, cdev);
	struct vchar_context_s *ctx;

	if (!node) {
		pr_err("cannot get char_dev_node_s from inode\n");
		return -EINVAL;
	}

	/* now get vchar ctx */
	ctx = (struct vchar_context_s *) dev_get_platdata(node->device);
	if (!ctx) {
		pr_err("cannot get vchar_context_s from driver data\n");
		return -EINVAL;
	}

	/* sanity check */
	if (ctx != filep->private_data) {
		pr_err("inode private not equal to filep private\n");
		return -EINVAL;
	}

	return vchar_close(ctx);
}

EXPORT_SYMBOL(vchar_close);

static int dev_check_flags(int f_flags)
{
/* Only these flags go into filep->f_flags. Other flags like O_CLOEXEC or
 * O_LARGEFILE does not, so we don't care about them
 */
	int relevant_flags = (O_APPEND | O_NONBLOCK | O_NDELAY | O_DIRECT | O_NOATIME);
	int supported_flags = O_NONBLOCK;

	f_flags &= relevant_flags;

	if (f_flags & ~supported_flags)
		return -EINVAL;
	return 0;
}

static const struct file_operations dev_fops = {
	.owner =	THIS_MODULE,
	.open =		dev_open,
	.read =		dev_read,
	.write =	dev_write,
	.poll =		dev_poll,
	.mmap =		dev_mmap,
	.unlocked_ioctl =
			dev_ioctl,
	.release =	dev_release,
	.llseek =	no_llseek,
	.check_flags =	dev_check_flags,
};

static int setup_mem(struct vchar_context_s *c)
{
	struct mem_region_s *mr;
	struct resource	*mem;
	size_t		smem;

	mem = platform_get_resource(c->vchar->pdev, IORESOURCE_MEM,
			VCHAR_NAME_AND_MMAP_MEMORY_RESOURCE);

	if (!mem || /* firstly check if there is a memory resource defined */
			/* secondly check if it is zero or non zero */
			((mem->start == 0) && ((mem->end == 0) ||
						(mem->end == (~0UL))))) {
		pr_err("dtb does not have shmem properly defined\n");
		return -ENODEV;
	}

	smem = mem->end - mem->start + 1;

	mr = kzalloc(sizeof(*mr), GFP_KERNEL);
	if (!mr) {
		return -ENOMEM;
	}

	mr->mem = mem->start;
	mr->len = smem;

	c->vchar->mem = mr;

	c->info.shmem_paddr = mr->mem;
	c->info.shmem_len = mr->len;
	pr_info("allocated mem successfully\n");
	return 0;
}

int vchar_setup_mem(struct vchar_context_s *c)
{
	struct resource *rtx, *rrx;
	struct resource *mem;
	void *rx, *tx;
	size_t srx, stx;

	mem = platform_get_resource(c->vchar->pdev, IORESOURCE_MEM,
			VCHAR_NAME_AND_MMAP_MEMORY_RESOURCE);
	if (mem == NULL) {
		pr_err("get Mem failed for device\n");
		return -EINVAL;
	}
	pr_info("probing device %s\n", mem->name);

	/* set device name */
	strncpy(c->name, mem->name, MAX_DEVICE_NAME);

	/* setup SHMEM */
	if (setup_mem(c)) {
		pr_warn("vchar %s does not have assigned mem\n",
				c->name);
	}

	rtx = platform_get_resource(c->vchar->pdev, IORESOURCE_MEM,
			VCHAR_TX_MEMORY_RESOURCE);
	rrx = platform_get_resource(c->vchar->pdev, IORESOURCE_MEM,
			VCHAR_RX_MEMORY_RESOURCE);

	if (!rtx || !rrx || (strcmp(rtx->name, "tx") != 0) ||
			(strcmp(rrx->name, "rx") != 0)) {
		pr_err("dtb does not have tx or rx properly defined\n");
		return -ENODEV;
	}

	/* +1 due start and end are valid addresses */
	stx =  rtx->end - rtx->start + 1;
	tx = ioremap_cache(rtx->start, stx);

	if (!stx || !tx) {
		pr_err("could not setup tx pipe memory\n");
		return -ENODEV;
	}

	/* setup static tx vchar information */
	c->info.tx_paddr = rtx->start;
	c->info.tx_kvaddr = (size_t) tx;
	c->info.tx_len = stx;

	/* +1 due start and end are valid addresses */
	srx = rrx->end - rrx->start + 1;
	rx = ioremap_cache(rrx->start, srx);

	if (!srx || !rx) {
		pr_err("could not setup rx pipe memory\n");
		vchar_cleanup_vring(c);
		return -ENODEV;
	}

	/* setup static rx vchar information */
	c->info.rx_paddr = rrx->start;
	c->info.rx_kvaddr = (size_t) rx;
	c->info.rx_len = srx;

	return 0;
}

void vchar_cleanup_mem(struct vchar_context_s *c)
{
	struct vchar_info *info = &c->info;

	if (info->tx_kvaddr) {
		/* conversion since kvaddr is uint64_t */
		size_t p = (size_t) info->tx_kvaddr;
		/* unmap tx region */
		iounmap((void *) p);
		pr_info("tx unmap done\n");
	}

	if (info->rx_kvaddr) {
		/* conversion since kvaddr is uint64_t */
		size_t p = (size_t) info->rx_kvaddr;
		/* unmap tx region */
		iounmap((void *) p);
		pr_info("rx unmap done\n");
	}
}

static irqreturn_t handler_func(int irq, void *args)
{
	struct vchar_context_s *ctx = (struct vchar_context_s *) args;
	if (!ctx) {
		pr_err("no context\n");
		return IRQ_NONE;
	}

	if (virtual_ring_check_ready(ctx->vring)) {
		pr_err("vring is not ready\n");
		return IRQ_NONE;
	}

	wake_up(&ctx->vchar->dev->rx_wq);

	ctx->info.rx_int_ctr++;

	return IRQ_HANDLED;
}

static int send_avail_thread(void *args)
{
	struct vchar_context_s *ctx = (struct vchar_context_s *) args;

	if (!ctx) {
		pr_err("no context\n");
		return -ENODEV;
	}

	while (!kthread_should_stop()) {
		if (!virtual_ring_check_ready(ctx->vring)) {
			/* transport ready */
			if (virtual_ring_send_get_avail(ctx->vring) > 0) {
				wake_up(&ctx->vchar->dev->tx_wq);
				wait_event_interruptible_timeout(
					ctx->vchar->dev->thread_wq,
					virtual_ring_send_get_avail(ctx->vring)
					== 0,
					10);
			} else {
				wait_event_interruptible_timeout(
					ctx->vchar->dev->thread_wq,
					virtual_ring_send_get_avail(ctx->vring)
					> 0,
					1);
			}

		} else
			wait_event_interruptible_timeout(
				ctx->vchar->dev->thread_wq,
				virtual_ring_check_ready(ctx->vring) == 0,
				10);
	}

	return 0;
}

static void cleanup_char_device(struct vchar_context_s *c)
{
	if (c) {
		struct char_dev_context_s *ctx = c->vchar->dev;

		if (ctx) {
			if (ctx->irq != VCHAR_NO_IRQ_SUPPORT)
				free_irq(ctx->irq, c);

			if (ctx->thread)
				kthread_stop(ctx->thread);

			if (ctx->node) {
				if (ctx->node->class)
					class_destroy(ctx->node->class);

				if (ctx->node->cdev_initialized)
					cdev_del(&ctx->node->cdev);

				if (ctx->node->major)
					unregister_chrdev_region(ctx->node->major,
							MINORS_PER_DEVICE);

				/* decrement a reference count of the module */
				module_put(ctx->node->cdev.owner);
				kfree(ctx->node);
			}

			/* zeroing */
			memset(ctx, 0, sizeof(*ctx));
			kfree(ctx);
			pr_info("cleanup done\n");
			c->vchar->dev = NULL;
		}
	}
}

static struct char_dev_node_s *alloc_device_node(struct vchar_context_s *c)
{
	struct char_dev_node_s *node;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		return NULL;
	}

	snprintf(node->class_name, MAX_DEVICE_NAME, "vchar-%s", c->name);

	if (alloc_chrdev_region(&node->major, 0, MINORS_PER_DEVICE, c->vchar->dev->dev_name)) {
		pr_err("failed to alloc region\n");
		kfree(node);
		return NULL;
	}

	cdev_init(&node->cdev, &dev_fops);
	node->cdev.owner = THIS_MODULE;

	node->class = class_create(THIS_MODULE, node->class_name);
	if (IS_ERR(node->class)) {
		pr_err("failed to register device class\n");
		kfree(node);
		return NULL;
	}

	node->device = device_create(node->class, NULL, MKDEV(MAJOR(node->major), 0),
			NULL, c->vchar->dev->dev_name);
	if (IS_ERR(node->device)) {
		pr_err("failed to create the device\n");
		kfree(node);
		return NULL;
	}

	if (cdev_add(&node->cdev, node->major, MINORS_PER_DEVICE)) {
		pr_err("failed to add cdev\n");
		kfree(node);
		return NULL;
	}

	node->cdev_initialized = 1;

	/*
	 * vchar device node can be used by an application.
	 * Increment a reference count of the module to prevent from module unloading.
	 */
	if (!try_module_get(node->cdev.owner))
		pr_err("failed to increment reference count to the module\n");

	return node;
}

/* setup and cleanup related functions */
static int setup_char_device(struct vchar_context_s *c)
{
	struct char_dev_context_s	*ctx;
	struct resource			*irq;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		return -ENOMEM;
	}

	snprintf(ctx->dev_name, MAX_DEVICE_NAME, "vchar-%s", c->name);

	/* assign c->dev used for cleanup in error path */
	c->vchar->dev = ctx;

	/* now setup IRQ and IPCG */
	/* get IPCGate optional parameter */
	if (of_property_read_u32(c->vchar->pdev->dev.of_node, "irqcap",
				&ctx->irqcap)) {
		/* If in DTS configuration there is no irqcap parameter
		 * it means that the driver will send data to opposite side
		 * without IRQ notification.
		*/
		ctx->irqcap = VCHAR_NO_NOTIFY_SUPPORT;
		pr_info("vchar notifications after send are disabled\n");
	}

	/* get IRQ optional parameter. If in DTS configuration there is no IRQ
	 * parameter it means that the driver is going to work in polling mode
	 * without receiving IRQs.
	*/
	irq = platform_get_resource(c->vchar->pdev, IORESOURCE_IRQ,
			VCHAR_IRQ_RESOURCE);

	if (c->flags & VIRTUAL_CHAR_DISABLE_NODE) {
		pr_info("vchar works in KERNEL mode\n");
	} else {
		pr_info("vchar uses device node\n");
		ctx->node = alloc_device_node(c);
		if (!ctx->node) {
			pr_err("failed to allocate device node for VCHAR\n");
			cleanup_char_device(c);
			return -ENODEV;
		}
	}

	if (c->flags & VIRTUAL_CHAR_NONBLOCK_MODE) {
		pr_info("vchar works in non-blocking mode.\n");
	} else {
		pr_info("vchar works in blocking mode.\n");
	}

	/* setup poll rx wq */
	init_waitqueue_head(&ctx->rx_wq);
	/* setup poll tx wq */
	init_waitqueue_head(&ctx->tx_wq);
	/* setup thread wq */
	init_waitqueue_head(&ctx->thread_wq);

	/* create send poll assistant thread */
	ctx->thread = kthread_run(send_avail_thread, c, ctx->dev_name);
	if (ctx->thread == NULL) {
		pr_err("failed to create send poll helper thread\n");
		cleanup_char_device(c);
		return -ENODEV;
	}

	/* If IRQ parameter was found in DTS configuration do IRQ line setup */
	if (irq != NULL) {
		/* request IRQ handling */
		if (request_irq(irq->start, handler_func, 0, "vchar_irq", c)) {
			pr_err("failed to request irq handling\n");
			cleanup_char_device(c);
			return -ENODEV;
		}

		/* assign irq to be properly cleaned up later */
		ctx->irq = irq->start;
		pr_info("vchar uses IRQ for receiving notifications\n");
	} else {
		ctx->irq = VCHAR_NO_IRQ_SUPPORT;
		pr_info("vchar does not use IRQ for receiving notifications\n");
	}

	/* setup vchar information */
	c->info.irq_nr = ctx->irq;

	if (ctx->irqcap != VCHAR_NO_NOTIFY_SUPPORT) {
		c->info.irqcap_nr = ctx->irqcap;
		pr_info("vchar notifications after send are enabled\n");
	}

	if (!(c->flags & VIRTUAL_CHAR_DISABLE_NODE)) {
		/* setup done finally set platform info to character device */
		ctx->node->device->platform_data = c;
	}

	pr_info("registered character device successfully\n");
	return 0;
}

static void cleanup_mem(struct vchar_context_s *c)
{
	if (c) {
		if (c->vchar) {
			kfree(c->vchar->mem);
			c->vchar->mem = NULL;
			kfree(c->vchar);
			c->vchar = NULL;
		}

		pr_info("mem cleanup done\n");
		kfree(c);
	}
}

static uint32_t vchar_get_flags(struct vchar_context_s *c)
{
	uint32_t flags;

	/* get 'flags' optional parameter */
	if (of_property_read_u32(c->vchar->pdev->dev.of_node, "flags",
			&flags)) {
		/* If in DTS configuration there is no 'flags' parameter
		 * it means flag is 0 - memory barriers are enabled,
		 * cache invalidation/flushing are disabled.
		 */
		return 0;
	}

	return flags;
}

static int vchar_get_buffer_sizes(struct vchar_context_s *c)
{
	uint32_t rx_nr, tx_nr;

	if (c == NULL)
		return -EINVAL;

	/* Read rx_nr parameter */
	if (of_property_read_u32(c->vchar->pdev->dev.of_node, "rx_nr",
			&rx_nr)) {
		/* No rx_nr parameter found in DTS, set 0 to use default */
		rx_nr = 0;
		pr_warn("rx_nr not found on device tree. Default value will be used.\n");
	}

	/* Read tx_nr parameter */
	if (of_property_read_u32(c->vchar->pdev->dev.of_node, "tx_nr",
			&tx_nr)) {
		/* No tx_nr parameter found in DTS, set 0 to use default */
		tx_nr = 0;
		pr_warn("tx_nr not found on device tree. Default value will be used.\n");
	}

	/* Set values */
	c->info.tx_msg_nr = tx_nr;
	c->info.rx_msg_nr = rx_nr;
	return 0;
}

static int vchar_probe(struct platform_device *pdev)
{
	struct vchar_context_s *ctx;
	int ret;

	pr_info("enter\n");

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		return -ENOMEM;
	}

	ctx->vchar = kzalloc(sizeof(*ctx->vchar), GFP_KERNEL);
	if (!ctx->vchar) {
		kfree(ctx);
		return -ENOMEM;
	}

	ctx->vchar->ctx = ctx;
	ctx->vchar->pdev = pdev;

	ret = vchar_get_buffer_sizes(ctx);
	if (ret) {
		cleanup_mem(ctx);
		return ret;
	}

	ret = vchar_get_flags(ctx);

	/* for VRING flags it's two low bytes */
	ctx->info.flags = ret & 0xffff;

	/* for VCHAR flags it's two high bytes */
	ctx->flags = ret & 0xffff0000;

	/* Timeout for synchronisation with opposite side */
	ctx->info.open_timeout = VRING_INFINITE_TIMEOUT;

	ret = vchar_init(ctx);
	if (ret) {
		cleanup_mem(ctx);
		return ret;
	}

	/* setup char */
	if (setup_char_device(ctx)) {
		pr_err("critical vchar %s does not have assigned dev\n",
				ctx->name);
		vchar_cleanup_vring(ctx);
		cleanup_mem(ctx);
		return -ENODEV;
	}

	INIT_LIST_HEAD(&ctx->vchar->list);
	list_add(&ctx->vchar->list, &vchar_channels);

	/* at the end we put our driver data in pdev */
	platform_set_drvdata(pdev, ctx);

	/* register panic/reboot notifiers */
	vchar_setup_reboot_notifiers(ctx);

	pr_info("init done successfully\n");
	return 0;
}

static int vchar_remove(struct platform_device *pdev)
{
	struct vchar_context_s *ctx = platform_get_drvdata(pdev);

	if (!ctx) {
		pr_err("no vchar context for this pdev\n");
		return -EINVAL;
	}
	vchar_notify_func(ctx);

	cleanup_char_device(ctx);

	vchar_cleanup_vring(ctx);

	/* deregister panic/reboot notifiers */
	vchar_clean_reboot_notifiers(ctx);

	cleanup_mem(ctx);

	pr_info("device removed successfully\n");
	return 0;
}

static void vchar_shutdown(struct platform_device *pdev)
{
	pr_info("not implemented\n");
}

static const struct of_device_id vchar_dt_ids[] = {
	{ .compatible = "coqoshv,vchar", },
	{}
};
MODULE_DEVICE_TABLE(of, vchar_dt_ids);

static struct platform_driver vchar_driver = {
	.probe	= vchar_probe,
	.remove	= vchar_remove,
	.shutdown  = vchar_shutdown,
	.driver	= {
		.name	= DEFAULT_DEVICE_NAME,
		.of_match_table  = vchar_dt_ids,
	},
};
module_platform_driver(vchar_driver);

MODULE_AUTHOR("Qualcomm Technologies ");
MODULE_DESCRIPTION("Qualcomm Technologies VCHAR support");
MODULE_ALIAS("platform:vchar");
MODULE_LICENSE("GPL");
