/*
 * $FILE: kthread.h
 *
 * Kernel, Guest or Layer0 thread
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_KTHREAD_H_
#define _XM_ARCH_KTHREAD_H_

#include <irqs.h>
#include <arch/processor.h>
#include <arch/segments.h>
#include <arch/xm_def.h>
#include <arch/atomic.h>

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#define SETUP_KSTACK(_k, _startup, _f) do {   \
    _k->ctrl.kStack=(xm_u32_t *)&_k->kStack[CONFIG_KSTACK_SIZE-4]; \
    *--(_k->ctrl.kStack)=(xm_u32_t)&_k->kStack[CONFIG_KSTACK_SIZE-4]; \
    *--(_k->ctrl.kStack)=(xm_u32_t)_f; \
    *--(_k->ctrl.kStack)=0; \
    *--(_k->ctrl.kStack)=(xm_u32_t)_startup; \
} while (0)

#define KTHREAD_ARCH_INIT(cKThread, stack) do {	\
    __asm__ __volatile__ ("finit\n\t" ::); \
    cKThread->ctrl.g->kArch.tss.t.ss0=DS_SEL; \
    cKThread->ctrl.g->kArch.tss.t.sp0=(xmAddress_t)&k->kStack[CONFIG_KSTACK_SIZE]; \
    SetWp(); \
} while (0)

struct kThreadArch {
    xmAddress_t pgd;
    xm_u32_t cr0;
    xm_u32_t cr4;
    //   xm_u32_t dbreg[8];
    pseudoDesc_t gdtr;
    gdtDesc_t gdtTab[CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES];
    pseudoDesc_t idtr;
    genericDesc_t idtTab[IDT_ENTRIES];
    ioTss_t tss;
#if 0
    struct {
	xm_s32_t esp1;
	xm_s32_t ss1;
	xm_s32_t esp2;
	xm_s32_t ss2;
    } tss;
#endif

};

#endif
