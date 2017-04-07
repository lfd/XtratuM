/*
 * $FILE: xmhypercalls.h
 *
 * Generic hypercall definition
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _LIB_XM_HYPERCALLS_H_
#define _LIB_XM_HYPERCALLS_H_

#ifdef _XM_KERNEL_
#error Guest file, do not include.
#endif

#include <xm_inc/config.h>
#include <xm_inc/hypercalls.h>
#include <xm_inc/arch/irqs.h>
#include <arch/xmhypercalls.h>

#ifndef __ASSEMBLY__
#include <xm_inc/arch/arch_types.h>
#include <xm_inc/objdir.h>

/* <track id="hypercall-list"> */
// Time management hypercalls
extern __stdcall xm_s32_t XM_get_time(xm_u32_t clock_id, xmTime_t *time);
extern __stdcall xm_s32_t XM_set_timer(xm_u32_t clock_id, xmTime_t abstime, xmTime_t interval);

// Partition status hypercalls
extern __stdcall xm_s32_t XM_suspend_partition(xm_u32_t partition_id);
extern __stdcall xm_s32_t XM_resume_partition(xm_u32_t partition_id);
extern __stdcall xm_s32_t XM_shutdown_partition(xm_u32_t partition_id);
extern __stdcall xm_s32_t XM_reset_partition(xm_u32_t partition_id, xm_u32_t resetMode, xm_u32_t status);
extern __stdcall xm_s32_t XM_halt_partition(xm_u32_t partition_id);
extern __stdcall xm_s32_t XM_idle_self(void);

// system status hypercalls
extern __stdcall xm_s32_t XM_halt_system(void);
extern __stdcall xm_s32_t XM_reset_system(xm_u32_t resetMode);

// Object related hypercalls
//@ \void{<track id="object-hc">}
extern __stdcall xm_s32_t XM_read_object(xmObjDesc_t objDesc, void *buffer, xm_u32_t size, xm_u32_t *flags);
extern __stdcall xm_s32_t XM_write_object(xmObjDesc_t objDesc, void *buffer, xm_u32_t size, xm_u32_t *flags);
extern __stdcall xm_s32_t XM_seek_object(xmObjDesc_t objDesc, xm_u32_t offset, xm_u32_t whence);
extern __stdcall xm_s32_t XM_ctrl_object(xmObjDesc_t objDesc, xm_u32_t cmd, void *arg);
//@ \void{</track id="object-hc">}

// Paging hypercalls
extern __stdcall xm_s32_t XM_set_page_type(xmAddress_t pAddr, xm_u32_t type);
extern __stdcall xm_s32_t XM_update_page32(xmAddress_t pAddr, xm_u32_t val);

extern __stdcall void XM_lazy_set_page_type(xmAddress_t pAddr, xm_u32_t type);
extern __stdcall void XM_lazy_update_page32(xmAddress_t pAddr, xm_u32_t val);

// Register hypercalls
extern __stdcall xm_s32_t XM_write_register32(xm_u32_t reg32, xm_u32_t val);
extern __stdcall xm_s32_t XM_write_register64(xm_u32_t reg64, xm_u32_t sreg64, xm_u32_t valH, xm_u32_t valL);

extern __stdcall void XM_lazy_write_register32(xm_u32_t reg32, xm_u32_t val);
extern __stdcall void XM_lazy_write_register64(xm_u32_t reg64, xm_u32_t sreg64, xm_u32_t valH, xm_u32_t valL);

// Hw interrupt management
extern __stdcall xm_s32_t XM_override_trap_hndl(xm_s32_t entry, struct trapHandler *);
extern __stdcall xm_s32_t XM_mask_hwirq(xm_u32_t noIrq);
extern __stdcall xm_s32_t XM_unmask_hwirq(xm_u32_t noIrq);
extern __stdcall xm_s32_t XM_raise_ipvi(xm_u32_t partition_id, xm_u8_t no_ipvi);

// Deferred hypercalls
extern __stdcall xm_s32_t XM_multicall(void *startAddr, void *endAddr);

/* </track id="hypercall-list"> */
#endif

#endif
