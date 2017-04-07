/*
 * $FILE: hypercalls.h
 *
 * Hypercalls definition
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_HYPERCALLS_H_
#define _XM_HYPERCALLS_H_

#ifdef _XM_KERNEL_
#include <arch/hypercalls.h>
#else
#include <xm_inc/arch/hypercalls.h>
#endif

/* <track id="abi-api-versions"> */
#define XM_ABI_VERSION 2
#define XM_ABI_SUBVERSION 0
#define XM_ABI_REVISION 0

#define XM_API_VERSION 2
#define XM_API_SUBVERSION 0
#define XM_API_REVISION 0
/* </track id="abi-api-versions"> */
// Generic hypercalls 

#define HYPERCALL_NOT_IMPLEMENTED (~0)

#define multicall_nr __MULTICALL_NR
#define halt_partition_nr __HALT_PARTITION_NR
#define suspend_partition_nr __SUSPEND_PARTITION_NR
#define resume_partition_nr __RESUME_PARTITION_NR
#define reset_partition_nr __RESET_PARTITION_NR
    #define XM_RESET_MODE 0x1
    #define XM_COLD_RESET 0x0
    #define XM_WARM_RESET 0x1
#define shutdown_partition_nr __SHUTDOWN_PARTITION_NR
#define halt_system_nr __HALT_SYSTEM_NR
#define reset_system_nr __RESET_SYSTEM_NR
#define idle_self_nr __IDLE_SELF_NR
#define write_register32_nr __WRITE_REGISTER32_NR
#define write_register64_nr __WRITE_REGISTER64_NR
#define get_time_nr __GET_TIME_NR
/* <track id="DOC-CLOCKS-AVAILABLE"> */
    #define XM_HW_CLOCK (0x0)
    #define XM_EXEC_CLOCK (0x1)
/* </track id="DOC-CLOCKS-AVAILABLE"> */
    #define XM_WATCHDOG_TIMER (0x2)

#define set_timer_nr __SET_TIMER_NR
#define read_object_nr __READ_OBJECT_NR
#define write_object_nr __WRITE_OBJECT_NR
#define seek_object_nr __SEEK_OBJECT_NR
    #define XM_OBJ_SEEK_CUR 0x0
    #define XM_OBJ_SEEK_SET 0x1
    #define XM_OBJ_SEEK_END 0x2
#define ctrl_object_nr __CTRL_OBJECT_NR
#define mask_hwirq_nr __MASK_HWIRQ_NR
#define unmask_hwirq_nr __UNMASK_HWIRQ_NR
#define update_page32_nr __UPDATE_PAGE32_NR
#define set_page_type_nr __SET_PAGE_TYPE_NR
#define override_trap_hndl_nr __OVERRIDE_TRAP_HNDL_NR
#define raise_ipvi_nr __RAISE_IPVI_NR

// Returning values

/* <track id="error-codes-list"> */
#define XM_OK                 (0)
#define XM_UNKNOWN_HYPERCALL (-1)
#define XM_INVALID_PARAM     (-2)
#define XM_PERM_ERROR        (-3)
#define XM_INVALID_CONFIG    (-4)
#define XM_INVALID_MODE      (-5)
#define XM_NOT_AVAILABLE     (-6)
#define XM_OP_NOT_ALLOWED    (-7)
#define XM_MULTICALL_ERROR   (-8)
/* </track id="error-codes-list"> */

#ifndef __ASSEMBLY__

#define HYPERCALL_TAB(_hc, _args) \
    __asm__ (".section .hypercallstab, \"a\"\n\t" \
	     ".align 4\n\t" \
	     ".long "#_hc"\n\t" \
	     ".previous\n\t" \
             ".section .hypercallflagstab, \"a\"\n\t" \
	     ".long ("#_args")\n\t" \
	     ".previous\n\t")

#endif

#endif
