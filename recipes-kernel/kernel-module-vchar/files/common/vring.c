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

#ifdef __LINUX__
#define pr_fmt(fmt) KBUILD_MODNAME "[%s]:%u " fmt, __func__, __LINE__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#endif

#include <vring.h>

/* macro helpers for computing sizes and offsets */
#define VIRTUAL_RING_HEAD_SIZE(nr) (sizeof(struct virtual_ring_pipe_s) \
	+ (sizeof(uint16_t) * nr))
#define VIRTUAL_RING_MESSAGE_SIZE(mem_size, nr) \
	((mem_size - VIRTUAL_RING_HEAD_SIZE(nr)) / nr)
#define VIRTUAL_RING_BLOCK_OFFSET(buffer, bs, nr) \
	(buffer + bs * nr)

#define test_not_exp_2(x) (x & (x-1))

int virtual_ring_setup(
		struct virtual_ring_s	**p,
		void			*tx_region,
		uint32_t		tx_len,
		uint32_t		tx_nr,
		void			*rx_region,
		uint32_t		rx_len,
		uint32_t		rx_nr,
		uint32_t		flags)
{
	struct virtual_ring_s *tmp;

	if (unlikely(!p || !tx_region || !tx_len || !rx_region || !rx_len ||
				!tx_nr || !rx_nr ||
				test_not_exp_2(tx_nr) ||
				test_not_exp_2(rx_nr))) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	tmp = ixcf_alloc(sizeof(*tmp));
	if (unlikely(!tmp)) {
		pr_err("memory allocation failed\n");
		return -ENOMEM;
	}

	tmp->tx = tx_region;
	tmp->tx_len = tx_len;
	tmp->tx_nr = tx_nr;

	tmp->rx = rx_region;
	tmp->rx_len = rx_len;
	tmp->rx_nr = rx_nr;
	tmp->rx_buffer_block_size = VIRTUAL_RING_MESSAGE_SIZE(rx_len,
			rx_nr);

	tmp->rx_buffer = (uint8_t *)((uintptr_t)tmp->rx  +
			VIRTUAL_RING_HEAD_SIZE(tmp->rx_nr));

	memset(tmp->tx, 0, tx_len);
	tmp->tx->buffer_blocks_number = tmp->tx_nr;
	tmp->tx->buffer_block_size = VIRTUAL_RING_MESSAGE_SIZE(tx_len,
			tmp->tx->buffer_blocks_number);

	memset(tmp->tx->buffer_size, 0,
			sizeof(uint16_t) * tmp->tx_nr);

	tmp->tx_buffer = (uint8_t *)((uintptr_t)tmp->tx  +
			VIRTUAL_RING_HEAD_SIZE(tmp->tx_nr));

	pr_info("MTU=%d Nr=%u\n", tmp->tx->buffer_block_size,
			tmp->tx->buffer_blocks_number);

	tmp->flags = flags;

	if (flags & VIRTUAL_RING_DISABLE_BARRIERS)
		pr_info("vring does not use memory barriers\n");
	else
		pr_info("vring uses memory barriers\n");

	if (flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH)
		pr_info("vring uses dcache flushing\n");
	else
		pr_info("vring does not use dcache flushing\n");

	if (flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		pr_info("vring uses dcache invalidation\n");
	else
		pr_info("vring does not use dcache invalidataion\n");

	*p = tmp;
	pr_info("successfully initialized\n");
	return 0;
}

int virtual_ring_cleanup(struct virtual_ring_s **p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	/* zeroing memory to avoid our state access */
	ixcf_memset(*p, 0, sizeof(**p));
	ixcf_free(*p);
	*p = NULL;
	return 0;
}

int virtual_ring_check_ready(struct virtual_ring_s *p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		rmb(); /* rx magic ready to read */

	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		/* do cache invalidation of rx my_magic memory */
		ixcf_invalidate_memory_region(
				&p->rx->my_magic,
				sizeof(p->rx->my_magic));

	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL) {
		/* do cache invalidation of rx magic memory */
		ixcf_invalidate_memory_region(
				&p->rx->magic,
				sizeof(p->rx->magic));
		ixcf_invalidate_memory_region(
				&p->rx->buffer_blocks_number,
				sizeof(p->rx->buffer_blocks_number));
		ixcf_invalidate_memory_region(
				&p->rx->buffer_block_size,
				sizeof(p->rx->buffer_block_size));
	}

	if (likely((p->tx->my_magic == p->rx->magic) &&
			(p->rx->my_magic == p->tx->magic))) {
		if (unlikely((p->rx->buffer_blocks_number != p->rx_nr) ||
					(p->rx->buffer_block_size !=
					 p->rx_buffer_block_size))) {
			pr_err("blocks number/size are not equal\n");
			return -EFAULT;
		}
		return 0;
	} else
		return -EAGAIN;
}

int virtual_ring_resync(struct virtual_ring_s *p)
{
	uint32_t magic;

	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	do {
		magic = virtual_ring_get_magic();
	} while (magic == 0);

	p->tx->my_magic = magic;

	ixcf_atomic_set(&p->tx->reader, 0);
	ixcf_atomic_set(&p->tx->writer, 0);

	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		wmb(); /* populate my_magic ready to read */

	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		/* flush my_magic */
		ixcf_flush_memory_region(&p->tx->my_magic,
				sizeof(p->tx->my_magic));

	return 0;
}

int virtual_ring_sync(struct virtual_ring_s *p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	/* do cache invalidation of rx writer memory */
	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		ixcf_invalidate_memory_region(
			&p->rx->my_magic,
			sizeof(p->rx->my_magic));

	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		rmb(); /* rx magic ready to read */

	if (unlikely(p->rx->my_magic != p->tx->magic)) {
		pr_err_once("Magic value was changed on the other side, "
			    "peer has probably restarted, reset our side\n");
		ixcf_atomic_set(&p->tx->reader, 0);
		ixcf_atomic_set(&p->tx->writer, 0);
		ixcf_atomic_set(&p->tx->reading, 0);

		/* sync oposit side magic */
		p->tx->magic = p->rx->my_magic;

		if (p->flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH)
			ixcf_flush_memory_region(&p->tx->magic,
						 sizeof(p->tx->magic));
	}
	return 0;
}

int virtual_ring_get_tx_message_size(struct virtual_ring_s *p)
{
	if (unlikely(!p || !p->tx)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	return p->tx->buffer_block_size;
}

int virtual_ring_get_rx_message_size(struct virtual_ring_s *p)
{
	if (unlikely(!p || !p->rx)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (unlikely(p->rx->buffer_block_size != p->rx_buffer_block_size)) {
		pr_err("buffer blocks sizes not matching\n");
		return -EINVAL;
	}

	return p->rx_buffer_block_size;
}

int virtual_ring_validate_rx_messages_number(struct virtual_ring_s *p)
{
	if (unlikely(!p || !p->rx)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (unlikely(p->rx->buffer_blocks_number != p->rx_nr)) {
		pr_err("buffer blocks number not matching\n");
		return -EINVAL;
	}

	return 0;
}

int virtual_ring_send_get_avail(struct virtual_ring_s *p)
{
	int consumer, producer;

	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		rmb(); /* rx reader ready to read */

	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		ixcf_invalidate_memory_region(
				&p->rx->reader,
				sizeof(p->rx->reader));

	consumer = ixcf_atomic_read(&p->rx->reader);
	producer = ixcf_atomic_read(&p->tx->writer);

	/* check for overflow */
	if (producer < 0 || consumer < 0) {
		pr_err("overflow detected! consumer: %d producer: %d\n",
				consumer, producer);
		return -EOVERFLOW;
	}

	if (producer >= consumer)
		return p->tx->buffer_blocks_number -
			(producer - consumer);
	else
		return p->tx->buffer_blocks_number -
			(producer + (INT_MAX - consumer));
}

int virtual_ring_send_get_buffer(struct virtual_ring_s *p, uint8_t **ptr)
{
	int producer;
	int res;

	if (unlikely(!p || !ptr)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	/* initially set no buffer */
	*ptr = NULL;

	/* check if we are not in the other send progress */
	if (ixcf_atomic_cmpxchg(&p->tx_in_progress, 0, 1) != 0) {
		pr_err("invalid state, last send not done\n");
		return -EBUSY;
	}

	res = virtual_ring_send_get_avail(p);
	if (res <= 0) {
		/* error or no space left */
		ixcf_atomic_set(&p->tx_in_progress, 0);
		if (unlikely(res < 0))
			return res;
		return -EAGAIN;
	}

	producer = ixcf_atomic_read(&p->tx->writer) % p->tx->buffer_blocks_number;

	/* check for overflow */
	if (producer < 0) {
		pr_err("overflow detected! producer: %d\n", producer);
		return -EOVERFLOW;
	}

	*ptr = VIRTUAL_RING_BLOCK_OFFSET(p->tx_buffer, p->tx->buffer_block_size,
			producer);
	return 0;
}

int virtual_ring_send_cancel(struct virtual_ring_s *p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (unlikely(ixcf_atomic_read(&p->tx_in_progress) != 1)) {
		pr_err("invalid state, not in tx_in_progress state\n");
		return -EBUSY;
	}

	ixcf_atomic_set(&p->tx_in_progress, 0);

	return 0;
}

int virtual_ring_send(struct virtual_ring_s *p, uint16_t len)
{
	int producer;

	if (unlikely(!p || !len || (len > p->tx->buffer_block_size))) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (unlikely(ixcf_atomic_read(&p->tx_in_progress) != 1)) {
		pr_err("invalid state, not in tx_in_progress\n");
		return -EBUSY;
	}

	/* fetch index of ring cell used for storing message */
	producer = ixcf_atomic_read(&p->tx->writer) %
		p->tx->buffer_blocks_number;

	/* check for overflow */
	if (producer < 0) {
		pr_err("overflow detected! producer: %d\n", producer);
		return -EOVERFLOW;
	}

	/* set message size */
	p->tx->buffer_size[producer] = len;

	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		wmb(); /* flush data and size before index increase */

	/* now operate with caches if needed */
	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH) {
		uint8_t *ptr = VIRTUAL_RING_BLOCK_OFFSET(p->tx_buffer,
				p->tx->buffer_block_size, producer);
		/* flush size */
		ixcf_flush_memory_region(&p->tx->buffer_size[producer],
			sizeof(p->tx->buffer_size[producer]));
		/* flush buffer */
		ixcf_flush_memory_region(ptr, len);
	}

	/* shift write message index */
	ixcf_atomic_inc(&p->tx->writer);

	/* drop sending flag */
	ixcf_atomic_set(&p->tx_in_progress, 0);

	/* now operate with caches if needed */
	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH) {
		/* flush write head */
		ixcf_flush_memory_region(&p->tx->writer, sizeof(p->tx->writer));
	}

	return 0;
}

int virtual_ring_recv_get_pending(struct virtual_ring_s *p)
{
	int consumer, producer;

	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		rmb(); /* rx writer ready to read */

	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		/* do cache invalidation of rx writer memory */
		ixcf_invalidate_memory_region(
				&p->rx->writer,
				sizeof(p->rx->writer));

	producer = ixcf_atomic_read(&p->rx->writer);
	consumer = ixcf_atomic_read(&p->tx->reader);

	/* check for overflow */
	if (producer < 0 || consumer < 0) {
		pr_err("overflow detected! consumer: %d producer: %d\n",
				consumer, producer);
		return -EOVERFLOW;
	}

	if (producer >= consumer)
		return producer - consumer;
	else
		return producer + (INT_MAX - consumer);
}

int virtual_ring_recv(struct virtual_ring_s *p, uint8_t *ptr, uint32_t *len)
{
	int consumer, producer;

	if (unlikely(!p || !ptr || !len)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	producer = ixcf_atomic_read(&p->rx->writer);
	consumer = ixcf_atomic_read(&p->tx->reader);

	/* check for overflow */
	if (producer < 0 || consumer < 0) {
		pr_err("overflow detected! consumer: %d producer: %d\n",
				consumer, producer);
		return -EOVERFLOW;
	}

	if (consumer != producer) {
		int consumer_aligned = consumer % p->rx_nr;
		uint8_t *ptrRBO = VIRTUAL_RING_BLOCK_OFFSET(p->rx_buffer,
				p->rx_buffer_block_size,
				consumer_aligned);
		uint16_t *sz_ptr = &p->rx->buffer_size[consumer_aligned];
		uint16_t sz;

		if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
			rmb(); /* size and buffer ready to read */

		if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL) {
			/* do cache invalidation of size memory */
			ixcf_invalidate_memory_region(sz_ptr, sizeof(sz));
			/* do cache invalidation of data memory */
			sz = ixcf_read_once(*sz_ptr);
			ixcf_invalidate_memory_region(ptrRBO, sz);
		} else {
			sz = ixcf_read_once(*sz_ptr);
		}

		if (*len >= sz) {
			memcpy(ptr, ptrRBO, sz);
			*len = sz;

			if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
				rmb(); /* wait before shift reader */

			ixcf_atomic_inc(&p->tx->reader);

			if (p->flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH) {
				ixcf_flush_memory_region(
						&p->tx->reader,
						sizeof(p->tx->reader));
			}
			return 0;
		}
		return -ENOMEM;
	}
	return -EBUSY;
}

int virtual_ring_notify_flag_disable(struct virtual_ring_s *p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	ixcf_atomic_set(&p->tx->reading, 0);
	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		wmb(); /* flush reading flag */

	/* now operate with caches if needed */
	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH)
		/* flush reading flag */
		ixcf_flush_memory_region(&p->tx->reading,
				sizeof(p->tx->reading));

	return 0;
}

int virtual_ring_notify_flag_enable(struct virtual_ring_s *p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	ixcf_atomic_set(&p->tx->reading, 1);
	if (!(p->flags & VIRTUAL_RING_DISABLE_BARRIERS))
		wmb(); /* flush reading flag */

	/* now operate with caches if needed */
	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_FLUSH)
		/* flush reading flag */
		ixcf_flush_memory_region(&p->tx->reading,
				sizeof(p->tx->reading));

	return 0;
}

int virtual_ring_notify_flag_get(struct virtual_ring_s *p)
{
	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	if (p->flags & VIRTUAL_RING_ENABLE_CACHE_INVAL)
		/* do cache invalidation of size memory */
		ixcf_flush_memory_region(&p->rx->reading,
				sizeof(p->rx->reading));

	return ixcf_atomic_read(&p->rx->reading);
}

int virtual_ring_wait_opposite_interruptible_timeout(struct virtual_ring_s *p, long timeout)
{
	int res = 0;
	long ticks = 0;

	if (unlikely(!p)) {
		pr_err("invalid arguments\n");
		return -EINVAL;
	}

	res = virtual_ring_sync(p);
	if (res) {
		pr_err("cannot sync\n");
		return res;
	}

	res = virtual_ring_check_ready(p);
	while (res && (res != -EINVAL) && (res != -EFAULT)) {
		if (timeout != VRING_INFINITE_TIMEOUT) {
			if (ticks >= timeout || ticks < 0) {
				/* timeout happened */
				return -ETIMEDOUT;
			}
		}

		res = ixcf_sleep(VRING_STEP_TIMEOUT);
		if (res) {
			res = -EFAULT;
			pr_info("wait is interrupted\n");
			break;
		}

		if (timeout != VRING_INFINITE_TIMEOUT) {
			ticks += VRING_STEP_TIMEOUT;
		}

		/* sync magics and retry */
		res = virtual_ring_sync(p);
		if (res) {
			pr_err("cannot sync\n");
			break;
		}
		res = virtual_ring_check_ready(p);
	}
	return res;
}
