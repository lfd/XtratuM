/*
 * $FILE: hypercalls.h
 *
 * Processor-related hypercalls definition
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_HYPERCALLS_H_
#define _XM_ARCH_HYPERCALLS_H_

//@ \void{<track id="hypercall-numbers">}
#define __MULTICALL_NR 0
#define __HALT_PARTITION_NR 1
#define __SUSPEND_PARTITION_NR 2
#define __RESUME_PARTITION_NR 3
#define __RESET_PARTITION_NR 4
#define __SHUTDOWN_PARTITION_NR 5
#define __HALT_SYSTEM_NR 6
#define __RESET_SYSTEM_NR 7
#define __IDLE_SELF_NR 8
#define __WRITE_REGISTER32_NR 9
#define __GET_TIME_NR 10
#define __SET_TIMER_NR 11
#define __READ_OBJECT_NR 12
#define __WRITE_OBJECT_NR 13
#define __SEEK_OBJECT_NR 14
#define __CTRL_OBJECT_NR 15
#define __MASK_HWIRQ_NR 16
#define __UNMASK_HWIRQ_NR 17
#define __UPDATE_PAGE32_NR 18
#define __SET_PAGE_TYPE_NR 19
#define __WRITE_REGISTER64_NR 20
#define __OVERRIDE_TRAP_HNDL_NR 21
#define __RAISE_IPVI_NR 22
#define ia32_update_sys_struct_nr 23
#define ia32_set_idt_desc_nr 24
#define __SET_SPARE_GUEST_NR 25
//@ \void{</track id="hypercall-numbers">}

#define NR_HYPERCALLS 26

//@ \void{<track id="asm-hypercall-numbers">}

//@ \void{</track id="asm-hypercall-numbers">}

#endif
