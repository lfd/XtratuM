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

#ifndef _XM_ARCH_IRQS_H_
#define _XM_ARCH_IRQS_H_

#ifdef _XM_KERNEL_
#include <arch/xm_def.h>
#else
#include <xm_inc/arch/xm_def.h>
#endif

/* existing traps (just 32 in the ia32 arch) */
#define HWIRQ_NR CONFIG_NO_HWIRQS
#define TRAP_NR 32

/* <track id="hardware-exception-list"> */
#define  IA32_DIVIDE_ERROR               (0x00)  //@  \color{black} //  0
#define  IA32_RESERVED1_EXCEPTION        (0x01)  //@  \color{black} //  1
#define  IA32_NMI_INTERRUPT              (0x02)  //@  \color{black} //  2
#define  IA32_BREAKPOINT_EXCEPTION       (0x03)  //@  \color{black} //  3
#define  IA32_OVERFLOW_EXCEPTION         (0x04)  //@  \color{black} //  4
#define  IA32_BOUNDS_EXCEPTION           (0x05)  //@  \color{black} //  5
#define  IA32_INVALID_OPCODE             (0x06)  //@  \color{black} //  6
#define  IA32_COPROCESSOR_NOT_AVAILABLE  (0x07)  //@  \color{black} //  7
#define  IA32_DOUBLE_FAULT               (0x08)  //@  \color{black} //  8
#define  IA32_COPROCESSOR_OVERRRUN       (0x09)  //@  \color{black} //  9
#define  IA32_INVALID_TSS                (0x0a)  //@  \color{black} //  10
#define  IA32_SEGMENT_NOT_PRESENT        (0x0b)  //@  \color{black} //  11
#define  IA32_STACK_SEGMENT_FAULT        (0x0c)  //@  \color{black} //  12
#define  IA32_GENERAL_PROTECTION_FAULT   (0x0d)  //@  \color{black} //  13
#define  IA32_PAGE_FAULT                 (0x0e)  //@  \color{black} //  14
#define  IA32_RESERVED2_EXCEPTION        (0x0f)  //@  \color{black} //  15
#define  IA32_FLOATING_POINT_ERROR       (0x10)  //@  \color{black} //  16
#define  IA32_ALIGNMENT_CHECK            (0x11)  //@  \color{black} //  17
#define  IA32_MACHINE_CHECK              (0x12)  //@  \color{black} //  18
/* </track id="hardware-exception-list"> */

#ifndef __ASSEMBLY__
struct trapHandler {
    xm_u16_t cs;
    xmAddress_t eip;
};

#endif

#ifdef _XM_KERNEL_
#define IDT_ENTRIES 256

/* existing hw interrupts (required by PIC and APIC) */
#define FIRST_EXTERNAL_VECTOR 0x20
#define FIRST_USER_IRQ_VECTOR 0x30

#define UD_HNDL 0x6
#define NM_HNDL 0x7
#define GP_HNDL 0xd
#define PF_HNDL 0xe

#ifndef __ASSEMBLY__

typedef struct {
    // saved registers
    xm_s32_t ebx;
    xm_s32_t ecx;
    xm_s32_t edx;
    xm_s32_t esi;
    xm_s32_t edi;
    xm_s32_t ebp;
    xm_s32_t eax;
    xm_s32_t xds;
    xm_s32_t xes;
    xm_s32_t xfs;
    xm_s32_t xgs;
    // irq or trap raised
    xm_s32_t irqNr;
    // error code pushed by the processor, 0 if none
    xm_s32_t errorCode;
    // processor state frame
    xm_s32_t eip;
    xm_s32_t xcs;
    xm_s32_t eflags;
    xm_s32_t esp;
    xm_s32_t xss;
} irqCtxt_t;

typedef irqCtxt_t hypercallCtxt_t;

#if 0
typedef struct {
    // saved registers
    xm_s32_t ebx;
    xm_s32_t ecx;
    xm_s32_t edx;
    xm_s32_t esi;
    xm_s32_t edi;
    xm_s32_t ebp;
    xm_s32_t eax;
    xm_s32_t xds;
    xm_s32_t xes;
    xm_s32_t xfs;
    xm_s32_t xgs;
    // processor state frame
    xm_s32_t eip;
    xm_s32_t xcs;
    xm_s32_t eflags;
    xm_s32_t esp;
    xm_s32_t xss;
} hypercallCtxt_t;
#endif

#define GET_ECODE(ctxt) ctxt->errorCode
#define GET_ICTXT_PC(ctxt) ctxt->eip
#define GET_HCTXT_PC(ctxt) ctxt->eip

static inline xm_s32_t IsSvIrqCtxt(irqCtxt_t *ctxt) {    
    return (ctxt->xcs&0x3)?0:1;
}

extern void EarlySetupIrqs(void);
#endif
#endif

#endif

