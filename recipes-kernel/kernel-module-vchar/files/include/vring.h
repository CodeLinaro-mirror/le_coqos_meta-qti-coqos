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
#ifndef __COQOS_VIRTUAL_RING_H__
#define __COQOS_VIRTUAL_RING_H__

#ifdef __LINUX__
#include <linux/types.h>
#endif

/* Low 2 bytes are reserved for VRING flags */

/* Enforces virtual ring to do wmb and rmb in appropriate places */
#define VIRTUAL_RING_DISABLE_BARRIERS	0x0001
/* Enforces virtual ring flush dcache to keep memory consistent */
#define VIRTUAL_RING_ENABLE_CACHE_FLUSH	0x0002
/* Enforces virtual ring invalidate dcache to keep memory consistent */
#define VIRTUAL_RING_ENABLE_CACHE_INVAL	0x0004

/* shared memory structure representing unidirectional pipe */
#pragma pack(push, 1)
struct virtual_ring_pipe_s {
	/* initialization synchronization */
	/* this contains assigned by tx owner value */
	uint32_t	my_magic;
	/* session cookie for paired pipe */
	uint32_t	magic;
	/* written elements counter */
	atomic_t	writer;
	/* read elements counter */
	atomic_t	reader;
	/*
	 * flag indicates that reading under progress
	 * should be inc every interrupt/success poll
	 * and dec every success receive
	 */
	atomic_t	reading;
	/* size of buffer block element */
	uint16_t	buffer_block_size;
	/* number of buffer blocks */
	uint16_t	buffer_blocks_number;
	/* an array of message sizes */
	uint16_t	buffer_size[0];
};

#pragma pack(pop)

/* context structure includes 2 pipes and synchronization logic */
struct virtual_ring_s {
	/* rx pipe page pointer */
	uint32_t			rx_len;
	uint8_t				rx_nr;
	struct virtual_ring_pipe_s	*rx;
	/* this variable contains locally computed remote side block size */
	uint16_t			rx_buffer_block_size;
	/* tx pipe page pointer */
	uint32_t			tx_len;
	uint8_t				tx_nr;
	struct virtual_ring_pipe_s	*tx;

	atomic_t			tx_in_progress;

	/* specific operations depends on memory config */
	uint32_t			flags;

	/* Buffer blocks */
	uint8_t		*rx_buffer;
	uint8_t		*tx_buffer;
};

/*
 * virtual_ring_setup - initializes new instance of ipc mechanism
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 *	tx_region:	memory region used for send
 *	tx_len:		send region size
 *	tx_nr:		number of messages in tx ring (power of 2)
 *	rx_region:	memory region used for receive
 *	rx_len:		receive region size
 *	rx_nr:		number of messages in rx ring (power of 2)
 *	flags:		configure specific evidences (mb, cache)
 * returns:
 *	0:		success
 *	-EINVAL:	bad arguments
 *	-ENODEV:	coqoshv unavailable or cannot set irq
 *	-ENOMEM:	no resources with specified name
 * It is non blocking call.
 */
int virtual_ring_setup(
		struct virtual_ring_s	**p,
		void			*tx_region,
		uint32_t		tx_len,
		uint32_t		tx_nr,
		void			*rx_region,
		uint32_t		rx_len,
		uint32_t		rx_nr,
		uint32_t		flags);

/*
 * virtual_ring_cleanup - cleans up existing instance of ipc mechanism and
 *	frees used memory.
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_cleanup(struct virtual_ring_s **p);

/*
 * virtual_ring_ready - checks if oposit side ready to receive
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 *	-EBUSY:		oposit side is unavailable (rebooting)
 *	-EAGAIN:	synchronization needed or other side booting
 * It is non blocking call.
 */
int virtual_ring_check_ready(struct virtual_ring_s *p);

/*
 * virtual_ring_resync - is intended to do update of synchronization state and
 *	resynchronize with the oposit side of ipc channel. It should be called
 *	once when you initiate the connection or consider that oposit side was
 *	down and now brings up.
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_resync(struct virtual_ring_s *p);

/*
 * virtual_ring_sync - is intended to signal oposit side that we are alive
 *	through simple sync step, could be called at any time
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 *	-EBUSY:		oposit side is unavailable
 * It is non blocking call.
 */
int virtual_ring_sync(struct virtual_ring_s *p);

#define VRING_INFINITE_TIMEOUT 	LONG_MAX

/*
 * virtual_ring_wait_opposite_interruptible_timeout - blocks interruptible execution of
 *	current thread until oposit side has not done re-synchronization
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 *	timeout:	timeout value in jiffies
 *			VRING_INFINITE_TIMEOUT -  infinite timeout
 * returns:
 *	0:		success
 *	-ETIMEDOUT:	on timeout
 *	-EINVAL:	bad args
 *	-EFAULT:	waiting is interrupted
 * It it non blocking call if timeout is 0, otherwise it's a blocking call that
 * might be interrupted.
 */
int virtual_ring_wait_opposite_interruptible_timeout(struct virtual_ring_s *p, long timeout);

/*
 * virtual_ring_get_tx_message_size - returns ipc send message size
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	>0:		message size
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_get_tx_message_size(struct virtual_ring_s *p);

/*
 * virtual_ring_get_rx_message_size - returns ipc recv message size
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	>0:		message size
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_get_rx_message_size(struct virtual_ring_s *p);

/*
 * virtual_ring_validate_rx_messages_number - validates ipc recv messages number
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		arguments are valid
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_validate_rx_messages_number(struct virtual_ring_s *p);

/*
 * virtual_ring_send_get_avail - return number of ring cells available for
 *	send operation execution
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	> 0:		success number of cells
 *	0:		no space left
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_send_get_avail(struct virtual_ring_s *p);

/*
 * virtual_ring_send_get_buffer - returns ipc buffer (size is fixed) to be used
 *	for copying data in
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 *	ptr:		pointer to pointer for buffer
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 *	-EBUSY:		some other executer already stared send operation
 *	-EAGAIN:	no space left in ring
 * It is non blocking call.
 */
int virtual_ring_send_get_buffer(struct virtual_ring_s *p, uint8_t **ptr);

/*
 * virtual_ring_send_cancel - cancel previously arranged buffer
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 *	-EBUSY:		send already done or no get buffer before
 * It is non blocking call.
 */
int virtual_ring_send_cancel(struct virtual_ring_s *p);

/*
 * virtual_ring_send - signals ipc to issue send operation on previously
 *	allocated buffer
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 *	len:		length of utilized buffer space
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 *	-EBUSY:		double send (send already done or no get buffer before)
 * It is non blocking call.
 * NOTE: caller should do signaling by itself (if it is used), there is no
 * oposit side notification in this function
 */
int virtual_ring_send(struct virtual_ring_s *p, uint16_t len);

/*
 * virtual_ring_recv_get_pending - returns the number of messages to be
 *	consumed
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_recv_get_pending(struct virtual_ring_s *p);

/*
 * virtual_ring_recv - fills the buffer and len provided by a user
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 *	ptr:		buffer to put received data
 *	len:		pointer to len variable
 * returns:
 *	>= 0:		success number of pending messages
 *	-EINVAL:	bad args
 *	-EBUSY:		ring is empty
 * It is non blocking call.
 */
int virtual_ring_recv(struct virtual_ring_s *p, uint8_t *ptr, uint32_t *len);

/*
 * virtual_ring_notify_flag_disable - let the other side to know that it should
 * not send more notifications
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_notify_flag_disable(struct virtual_ring_s *p);

/*
 * virtual_ring_notify_flag_enable - let the other side to know that it should
 * start sending notifications
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		success
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_notify_flag_enable(struct virtual_ring_s *p);

/*
 * virtual_ring_notify_flag_get - read the state of oposite side recv
 * notification
 * args:
 *	p:		pointer to virtual_ring_s descriptor
 * returns:
 *	0:		disabled
 *	1:		enabled
 *	-EINVAL:	bad args
 * It is non blocking call.
 */
int virtual_ring_notify_flag_get(struct virtual_ring_s *p);

/* Cross platform functions are defined in platform related header file. */
#ifdef __LINUX__
#include "linux/vring.h"
#include "linux/ixcf.h"
#endif

#endif /* ifndef __COQOS_VIRTUAL_RING_H__ */
