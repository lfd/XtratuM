/*
 * $FILE: xmconf.h
 *
 * Config parameters for both, XM and partitions
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_XMCONF_H_
#define _XM_ARCH_XMCONF_H_

/* <track id="test-hw-events"> */

#define  XM_HM_EV_IA32_DIVIDE_EXCEPTION          (XM_HM_MAX_GENERIC_EVENTS + 0)
#define  XM_HM_EV_IA32_DEBUGGER_EXCEPTION        (XM_HM_MAX_GENERIC_EVENTS + 1)
#define  XM_HM_EV_IA32_NMI_EXCEPTION             (XM_HM_MAX_GENERIC_EVENTS + 2)
#define  XM_HM_EV_IA32_BREAKPOINT_EXCEPTION      (XM_HM_MAX_GENERIC_EVENTS + 3)
#define  XM_HM_EV_IA32_OVERFLOW_EXCEPTION        (XM_HM_MAX_GENERIC_EVENTS + 4)
#define  XM_HM_EV_IA32_BOUNDS_EXCEPTION          (XM_HM_MAX_GENERIC_EVENTS + 5)
#define  XM_HM_EV_IA32_INVALID_OPCODE            (XM_HM_MAX_GENERIC_EVENTS + 6)
#define  XM_HM_EV_IA32_COPROCESOR_UNAVAILABLE    (XM_HM_MAX_GENERIC_EVENTS + 7)
#define  XM_HM_EV_IA32_DOUBLE_FAULT              (XM_HM_MAX_GENERIC_EVENTS + 8)
#define  XM_HM_EV_IA32_COPROCESSOR_OVERRUN       (XM_HM_MAX_GENERIC_EVENTS + 9)
#define  XM_HM_EV_IA32_INVALID_TSS               (XM_HM_MAX_GENERIC_EVENTS + 10)
#define  XM_HM_EV_IA32_SEGMENT_NOT_PRESENT       (XM_HM_MAX_GENERIC_EVENTS + 11)
#define  XM_HM_EV_IA32_STACK_FAULT               (XM_HM_MAX_GENERIC_EVENTS + 12)
#define  XM_HM_EV_IA32_GENERAL_PROTECTION_FAULT  (XM_HM_MAX_GENERIC_EVENTS + 13)
#define  XM_HM_EV_IA32_PAGE_FAULT                (XM_HM_MAX_GENERIC_EVENTS + 14)
#define  XM_HM_EV_IA32_RESERVED                  (XM_HM_MAX_GENERIC_EVENTS + 15)
#define  XM_HM_EV_IA32_MATH_FAULT                (XM_HM_MAX_GENERIC_EVENTS + 16)
#define  XM_HM_EV_IA32_ALIGNMENT_CHECK           (XM_HM_MAX_GENERIC_EVENTS + 17)
#define  XM_HM_EV_IA32_MACHINE_CHECK             (XM_HM_MAX_GENERIC_EVENTS + 18)
#define  XM_HM_EV_IA32_FLOATING_POINT_EXCEPTION  (XM_HM_MAX_GENERIC_EVENTS + 19)

/* </track id="test-hw-events"> */

#define XM_HM_MAX_EVENTS (XM_HM_MAX_GENERIC_EVENTS + 20)

struct xmcHpvArch {
};

struct xmcPartitionArch {
};

#endif
