/*
 * $FILE: guest.h
 *
 * Guest shared info
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_GUEST_H_
#define _XM_ARCH_GUEST_H_

#ifdef _XM_KERNEL_
#include <arch/atomic.h>
#include <arch/processor.h>
#else
#include <xm_inc/arch/atomic.h>
#include <xm_inc/arch/processor.h>
#endif

// gctrl->iflags' arch bits
#define IFLAGS_ARCH_IOPL_BIT 16
#define IFLAGS_ARCH_IOPL_MASK (0x3<<(IFLAGS_ARCH_IOPL_BIT))

#define IFLAGS_ATOMIC_MASK (1<<(IFLAGS_ARCH_IOPL_BIT+4))

// [31..12] -> Page physical address

// XXX: this structure is visible from the guest
/*  <track id="doc-Partition-Control-Table-Arch"> */
struct pctArch {
    //xm_u32_t dbreg[8];
    pseudoDesc_t gdtr;
    pseudoDesc_t idtr;
    volatile xm_u32_t tr;
    volatile xm_u32_t cr4;
    volatile xm_u32_t cr3;
    volatile xm_u32_t cr2;
    volatile xm_u32_t cr0;
    struct {
	volatile xm_u32_t sAddr;
	volatile xm_u32_t eAddr;
    } atomicArea;
};
/*  </track id="doc-Partition-Control-Table-Arch"> */

typedef struct {
} pitArch_t;

#endif
