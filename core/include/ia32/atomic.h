/*
 * $FILE: atomic.h
 *
 * atomic operations
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_ATOMIC_H_
#define _XM_ARCH_ATOMIC_H_

typedef volatile xm_u32_t xmAtomic_t;

#define XMAtomicSet(atomicaddr,value) ((*(atomicaddr))=(value))
#define XMAtomicGet(atomicaddr) (*(atomicaddr))
#define XMAtomicClearMask(bitmask, addr) ((*(addr)) &= (~(bitmask)))
#define XMAtomicSetMask(bitmask, addr) ((*(addr)) |= (bitmask))

#endif
