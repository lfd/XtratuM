/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_ARCH_IRQS_H_
#define _XAL_ARCH_IRQS_H_

#define TRAPTAB_LENGTH 256
#define EXT_IRQ_VECTOR 224
#define HwSti() XM_enable_irqs()
#define HwCli() XM_disable_irqs()

#ifndef __ASSEMBLY__

typedef struct trapCtxt {
    xm_u32_t ebx;
    xm_u32_t ecx;
    xm_u32_t edx;
    xm_u32_t esi;
    xm_u32_t edi;
    xm_u32_t ebp;
    xm_u32_t eax;
    xm_u32_t ds;
    xm_u32_t es;
    xm_u32_t fs;
    xm_u32_t gs;
    xm_s32_t irqNr;
    xm_s32_t errorCode;
    xm_s32_t iFlags;
    xm_s32_t eip;
    xm_s32_t xcs;
    xm_s32_t eflags;
    xm_s32_t esp;
    xm_s32_t ss;
} trapCtxt_t;

#endif
#endif
