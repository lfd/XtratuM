/*
 * $FILE: gen_offsets.h
 *
 * ASM offsets, this file only can be included from asm-offset.c
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef __GEN_OFFSETS_H_
#define _GEN_OFFSETS_H_
#ifndef _GENERATE_OFFSETS_
#error Do not include this file
#endif

#include <arch/atomic.h>
#include <kthread.h>
#include <sched.h>

static inline void GenerateOffsets(void) {
    // struct __kthread
    
    DEFINE(kStack, offsetof(struct __kThread, kStack),);

    // gctrl_t
    DEFINE(iFlags, offsetof(partitionControlTable_t, iFlags),);
    
    // partitionInformationTable_t id
    DEFINE(id, offsetof(partitionInformationTable_t, id), );

    // gctrl_t
    DEFINE(iFlags, offsetof(partitionControlTable_t, iFlags),);

    // struct xmPartitionHdr
    DEFINE(partitionControlTable, offsetof(struct xmPartitionHdr, partitionControlTable),);
    DEFINE(partitionInformationTable, offsetof(struct xmPartitionHdr, partitionInformationTable),);

/*  // localSched_t
  DEFINE(cKThread, offsetof(localSched_t, cKThread),);
  DEFINE(fpuOwner, offsetof(localSched_t, fpuOwner),);

  // kthread_t
  DEFINE(ctrl, offsetof(kThread_t, ctrl),);
  
  // struct __kthread
  DEFINE(g, offsetof(struct __kThread, g),);
  DEFINE(kStack, offsetof(struct __kThread, kStack),);

  // struct guest
  DEFINE(partitionControlTable, offsetof(struct guest, partitionControlTable),);
  DEFINE(kArch, offsetof(struct guest, kArch),);

  // gctrl_t
  DEFINE(iFlags, offsetof(partitionControlTable_t, iFlags),);

  // xm_atomic_t
  DEFINE(val, offsetof(xmAtomic_t, val),);  

  // struct kthread_arch
  DEFINE(gTbr, offsetof(struct kThreadArch, gTbr),);
  DEFINE(fpuRegs, offsetof(struct kThreadArch, fpuRegs),);

  // struct  irqTabEntry
  DEFINE(handler, offsetof(struct irqTabEntry, handler), );
  DEFINE(data, offsetof(struct irqTabEntry, data), );
*/
}

#endif
