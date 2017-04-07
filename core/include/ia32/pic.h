/*
 * $FILE: pic.h
 *
 * The PC's PIC
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_PIC_H_
#define _XM_ARCH_PIC_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

// Without the use of the IO-APIC, the pic controlls interrupts
// from 0 to 15 (16 irqs)
#define PIC_IRQS 16

extern void InitPic (xm_u8_t master_base, xm_u8_t slave_base);

#endif
