/*
 * $FILE: hypercalls.c
 *
 * XM system calls definitions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xmhypercalls.h>
#include <xm_inc/hypercalls.h>
#include <hypervisor.h>

xm_hcall0r(halt_system);
xm_hcall1r(reset_system, xm_u32_t, resetMode);
xm_hcall1r(halt_partition, xm_u32_t, partitionId);
xm_hcall1r(suspend_partition, xm_u32_t, partitionId);
xm_hcall1r(resume_partition, xm_u32_t, partitionId);
xm_hcall3r(reset_partition, xm_u32_t, partitionId, xm_u32_t, resetMode, xm_u32_t, status);
xm_hcall1r(shutdown_partition, xm_u32_t, partitionId);
xm_hcall0r(idle_self);
xm_hcall2r(write_register32, xm_u32_t, reg32, xm_u32_t, val);
xm_hcall4r(write_register64, xm_u32_t, reg64, xm_u32_t, sreg64, xm_u32_t, valH, xm_u32_t, valL);
xm_hcall4r(read_object, xmObjDesc_t, objDesc, void *, buffer, xm_u32_t, size, xm_u32_t *, flags);
xm_hcall4r(write_object, xmObjDesc_t, objDesc, void *, buffer, xm_u32_t, size, xm_u32_t *, flags);
xm_hcall3r(ctrl_object, xmObjDesc_t, objDesc, xm_u32_t, cmd, void *, arg);
xm_hcall3r(seek_object, xmObjDesc_t, objDesc, xm_u32_t, offset, xm_u32_t, whence);
xm_hcall2r(get_time, xm_u32_t, clock_id, xmTime_t *, time);
xm_hcall1r(mask_hwirq, xm_u32_t, noIrq);
xm_hcall1r(unmask_hwirq, xm_u32_t, noIrq);
xm_hcall2r(update_page32, xmAddress_t, pAddr, xm_u32_t, val);
xm_hcall2r(set_page_type, xmAddress_t, pAddr, xm_u32_t, type);
xm_hcall3r(ia32_update_sys_struct, xm_u32_t, procStruct, xm_u32_t, val1, xm_u32_t, val2);
xm_hcall2r(ia32_set_idt_desc, xm_s32_t, entry, genericDesc_t *, desc);
xm_hcall2r(override_trap_hndl, xm_s32_t, entry, struct trapHandler *, handler);
xm_hcall2r(raise_ipvi, xm_u32_t, partition_id, xm_u8_t, no_ipvi);

__stdcall xm_s32_t XM_multicall(void *startAddr, void *endAddr) {
    xm_s32_t _r;
    _XM_HCALL2(startAddr, endAddr, multicall_nr, _r);
    return _r;
}

__stdcall xm_s32_t XM_set_timer(xm_u32_t clock_id, xmTime_t abstime, xmTime_t interval) {
    xm_s32_t _r;
    _XM_HCALL5(clock_id, abstime, abstime>>32, interval, interval>>32, set_timer_nr, _r);
    return _r;
}

xm_lazy_hcall3(ia32_update_sys_struct, xm_u32_t, procStruct, xm_u32_t, val1, xm_u32_t, val2);
