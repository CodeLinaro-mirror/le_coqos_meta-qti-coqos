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
#ifndef __COQOS_LINUX_VRING_H__
#define __COQOS_LINUX_VRING_H__

#define VRING_STEP_TIMEOUT	100

uint32_t virtual_ring_get_magic(void);

#endif /* __COQOS_LINUX_VRING_H__ */
