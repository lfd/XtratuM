/*
 * $FILE: hypercalls.c
 *
 * XM's hypercalls
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <physmm.h>
#include <gaccess.h>
#include <hypercalls.h>
#include <sched.h>

__hypercall xm_s32_t Ia32UpdateSysStructSys(hypercallCtxt_t *ctxt, xm_u32_t procStruct, xm_u32_t val1, xm_u32_t val2) {
    extern xm_s32_t (*UpdPrcStructHndlTab[NR_IA32_PROC_STRUCT])(hypercallCtxt_t *, xm_u32_t, xm_u32_t);
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    if (procStruct<NR_IA32_PROC_STRUCT&&UpdPrcStructHndlTab[procStruct]) {
	if (UpdPrcStructHndlTab[procStruct](ctxt, val1, val2)<0) return XM_INVALID_PARAM;
    } else
	return XM_INVALID_PARAM;
    return XM_OK;
}

static inline xm_s32_t IsGateDescValid(genericDesc_t *desc) {
    xm_u32_t base, seg;
    
    // Only trap gates supported
    if ((desc->low&0x1fe0)==0xf00) {
	base=(desc->low&0xFFFF0000)|(desc->high&0xFFFF);
	if (base>=CONFIG_XM_OFFSET) {
	    kprintf("[GateDesc] Base (0x%x) > XM_OFFSET\n", base);
	    return 0;
	}
	seg=desc->high>>16;
	if (!(seg&0x3)) {
	    kprintf("[GateDesc] ring (%d) can't be zero\n", seg&0x3);
	    return 0;
	}
	return 1;
    }

    return 0;
}

__hypercall xm_s32_t Ia32SetIdtDescSys(hypercallCtxt_t *ctxt, xm_s32_t entry, genericDesc_t *__gParam desc) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    if (__CheckGParam(ctxt, desc, sizeof(genericDesc_t))<0) 
	return XM_INVALID_PARAM;
    if (entry<FIRST_USER_IRQ_VECTOR||entry>=IDT_ENTRIES)
	return XM_INVALID_PARAM;

    if ((desc->low&(1<<DESC_P_BIT))&&!IsGateDescValid(desc))
	return XM_INVALID_PARAM;

    sched->cKThread->ctrl.g->kArch.idtTab[entry]=*desc;
    return XM_OK;
}

__hypercall xm_s32_t OverrideTrapHndlSys(hypercallCtxt_t *ctxt, xm_s32_t entry, struct trapHandler *__gParam handler) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    if (__CheckGParam(ctxt, handler, sizeof(struct trapHandler))<0) 
	return XM_INVALID_PARAM;
    if (entry>TRAP_NR)
	return XM_INVALID_PARAM;

    sched->cKThread->ctrl.g->overrideTrapTab[entry].cs=handler->cs;
    sched->cKThread->ctrl.g->overrideTrapTab[entry].eip=handler->eip;

    return XM_OK;
}

// Hypercall table
HYPERCALL_TAB(MulticallSys, 0);
HYPERCALL_TAB(HaltPartitionSys, 1);
HYPERCALL_TAB(SuspendPartitionSys, 1);
HYPERCALL_TAB(ResumePartitionSys, 1);
HYPERCALL_TAB(ResetPartitionSys, 1);
HYPERCALL_TAB(ShutdownPartitionSys, 1);
HYPERCALL_TAB(HaltSystemSys, 1);
HYPERCALL_TAB(ResetSystemSys, 1);
HYPERCALL_TAB(IdleSelfSys, 0);
HYPERCALL_TAB(WriteRegister32Sys, 2);
HYPERCALL_TAB(GetTimeSys, 2);
HYPERCALL_TAB(SetTimerSys, 3);
HYPERCALL_TAB(ReadObjectSys, 4);
HYPERCALL_TAB(WriteObjectSys, 4);
HYPERCALL_TAB(SeekObjectSys, 3);
HYPERCALL_TAB(CtrlObjectSys, 3);
HYPERCALL_TAB(MaskHwIrqSys, 1);
HYPERCALL_TAB(UnmaskHwIrqSys, 1);
HYPERCALL_TAB(UpdatePage32Sys, 2);
HYPERCALL_TAB(SetPageTypeSys, 2);
HYPERCALL_TAB(WriteRegister64Sys, 4);
HYPERCALL_TAB(OverrideTrapHndlSys, 2);
HYPERCALL_TAB(RaiseIPVISys, 2);
HYPERCALL_TAB(Ia32UpdateSysStructSys, 3);
HYPERCALL_TAB(Ia32SetIdtDescSys, 2);
#ifdef CONFIG_SPARE_SCHEDULING
HYPERCALL_TAB(SetSpareGuestSys, 3);
#endif
