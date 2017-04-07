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
#include <kthread.h>
#include <gaccess.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <hypercalls.h>
#include <virtmm.h>
#include <vmmap.h>

__hypercall xm_s32_t MulticallSys(hypercallCtxt_t *ctxt, void *__gParam startAddr, void *__gParam endAddr) {
#define BATCH_GET_PARAM(_addr, _arg) *(xm_u32_t *)((_addr)+sizeof(xm_u32_t)*(2+(_arg)))
    localSched_t *sched=GET_LOCAL_SCHED();
    extern xm_s32_t (*hypercallsTab[NR_HYPERCALLS])(hypercallCtxt_t *ctxt, ...);
    extern struct {
	xm_u32_t noArgs;
    } hypercallFlagsTab[NR_HYPERCALLS];
    xmAddress_t addr;
    xm_u32_t noHyp;

    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());

    if (endAddr<startAddr)
	return XM_INVALID_PARAM;

    if (__CheckGParam(ctxt, startAddr, (xmAddress_t)endAddr-(xmAddress_t)startAddr)<0) 
	return XM_INVALID_PARAM;
    
    for (addr=(xmAddress_t)startAddr; addr<(xmAddress_t)endAddr;) {
	noHyp=*(xm_u32_t *)addr;
	*(xm_u32_t *)(addr+sizeof(xm_u32_t))&=~(0xffff<<16);
	if ((noHyp>=NR_HYPERCALLS)||(*(xm_u32_t *)(addr+sizeof(xm_u32_t))!=hypercallFlagsTab[noHyp].noArgs)) {
	    *(xm_u32_t *)(addr+sizeof(xm_u32_t))|=(XM_INVALID_PARAM<<16);
	    return XM_MULTICALL_ERROR;
	}
	*(xm_u32_t *)(addr+sizeof(xm_u32_t))|=hypercallsTab[noHyp](ctxt, BATCH_GET_PARAM(addr, 0), BATCH_GET_PARAM(addr, 1), BATCH_GET_PARAM(addr, 2), BATCH_GET_PARAM(addr, 3), BATCH_GET_PARAM(addr, 4))<<16;
	addr+=(hypercallFlagsTab[noHyp].noArgs+2)*sizeof(xm_u32_t);
//#ifdef EXPERIMENTAL
//	HwSti();
//	DoNop();
//	HwCli();
//#endif
    }

#undef BATCH_GET_PARAM
    return XM_OK;
}

__hypercall xm_s32_t HaltPartitionSys(hypercallCtxt_t *ctxt, xm_s32_t partitionId) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    if (partitionId!=sched->cKThread->ctrl.g->cfg->id) {
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;
    
	if ((partitionId<0)||(partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
	    return XM_INVALID_PARAM;
    
	kprintf("[HYPERCALL] (0x%x) Partition %d halted\n", sched->cKThread->ctrl.g->cfg->id, partitionId);
    
	HALT_PARTITION(partitionId);
	return XM_OK;
    }

    kprintf("[HYPERCALL] (0x%x) Halted\n", sched->cKThread->ctrl.g->cfg->id);
    HALT_PARTITION(partitionId);
    Scheduling();
    SystemPanic(0, ctxt, "[HYPERCALL] A halted partition is being executed");

    return XM_OK;
}

__hypercall xm_s32_t HaltSystemSys(hypercallCtxt_t *ctxt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    extern void HaltSystem(void);
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	return XM_PERM_ERROR;

    HwCli();
    HaltSystem();
    return XM_OK;
}

// XXX: the ".data" section is restored during the initialisation
__hypercall xm_s32_t ResetSystemSys(hypercallCtxt_t *ctxt, xm_u32_t resetMode) {
    extern xm_u32_t sysResetCounter[];
    extern void start(void);
    localSched_t *sched=GET_LOCAL_SCHED();

    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	return XM_PERM_ERROR;

    sysResetCounter[0]++;
    if ((resetMode&XM_RESET_MODE)==XM_WARM_RESET) {
	start();
    } else { // Cold Reset
	void (*startRSw)(void)=(void (*)(void))xmcTab.rsw.entryPoint;
	startRSw();
    }
    return XM_OK;
}

__hypercall xm_s32_t SuspendPartitionSys(hypercallCtxt_t *ctxt, xm_u32_t partitionId) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    if (partitionId!=sched->cKThread->ctrl.g->cfg->id) {
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;
	if ((partitionId<0||partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
	    return XM_INVALID_PARAM;
	if(IS_KTHREAD_FLAG_SET(partitionTab[partitionId], KTHREAD_HALTED_F))
		return XM_OP_NOT_ALLOWED;

	CLEAR_KTHREAD_FLAG(partitionTab[partitionId], KTHREAD_READY_F);
	return XM_OK;
    }
    
    CLEAR_KTHREAD_FLAG(sched->cKThread, KTHREAD_READY_F);
    Scheduling();
    return XM_OK;
}

__hypercall xm_s32_t ResumePartitionSys(hypercallCtxt_t *ctxt, xm_u32_t partitionId) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());

    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	return XM_PERM_ERROR;

    if ((partitionId<0||partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
	return XM_INVALID_PARAM;

    SET_KTHREAD_FLAG(partitionTab[partitionId], KTHREAD_READY_F);
    return XM_OK;
}

__hypercall xm_s32_t ResetPartitionSys(hypercallCtxt_t *ctxt, xm_u32_t partitionId, xm_u32_t resetMode, xm_u32_t status) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());

    if ((partitionId<0||partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
	return XM_INVALID_PARAM;

    if (partitionId!=sched->cKThread->ctrl.g->cfg->id)
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;
    
    return (ResetPartition(partitionTab[partitionId], ((resetMode&XM_RESET_MODE)==XM_COLD_RESET)?1:0, status)==0)?XM_OK:XM_INVALID_CONFIG;
}

__hypercall xm_s32_t ShutdownPartitionSys(hypercallCtxt_t *ctxt, xm_u32_t partitionId) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    if (partitionId!=sched->cKThread->ctrl.g->cfg->id) {
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;
	if ((partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
	    return XM_INVALID_PARAM;
    }

    SHUTDOWN_PARTITION(partitionId);
    return XM_OK;
}

__hypercall xm_s32_t IdleSelfSys(hypercallCtxt_t *ctxt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
#ifdef CONFIG_SPARE_SCHEDULING
    if (IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SPARE_GUEST_F)) {
        SetSpareGuest(NULL);
    }
#endif
    SET_KTHREAD_FLAG(sched->cKThread, KTHREAD_YIELD_F);
    Scheduling();
    return XM_OK;
}

__hypercall xm_s32_t WriteRegister32Sys(hypercallCtxt_t *ctxt, xm_u32_t reg32, xm_u32_t val) {
    extern xm_s32_t (*Reg32HndlTab[NR_REGS32])(hypercallCtxt_t *, xm_u32_t);
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    if (reg32<NR_REGS32&&Reg32HndlTab[reg32]) {
	if (Reg32HndlTab[reg32](ctxt, val)<0) return XM_INVALID_PARAM;
    } else 
	return XM_INVALID_PARAM;
    return XM_OK;
}

__hypercall xm_s32_t WriteRegister64Sys(hypercallCtxt_t *ctxt, xm_u32_t reg64, xm_u32_t sreg64, xm_u32_t valH, xm_u32_t valL) {
    extern xm_s32_t (*Reg64HndlTab[NR_REGS64])(hypercallCtxt_t *, xm_u32_t, xm_u32_t, xm_u32_t);
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    if (reg64<NR_REGS64&&Reg64HndlTab[reg64]) {
	if (Reg64HndlTab[reg64](ctxt, sreg64, valH, valL)<0) return XM_INVALID_PARAM;
    } else 
	return XM_INVALID_PARAM;
    return XM_OK;
}

__hypercall xm_s32_t SetTimerSys(hypercallCtxt_t *ctxt, xm_u32_t clockId, xmTime_t abstime, xmTime_t interval) {
    localSched_t *sched=GET_LOCAL_SCHED();

    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());

    if (abstime<0)
	return XM_INVALID_PARAM;

    // Disarming a timer
    if (!abstime) {
	switch(clockId) {
	case XM_HW_CLOCK:
	    DisarmKTimer(sched->cKThread->ctrl.g->kTimer);
	    return XM_OK;
	    break;
	case XM_EXEC_CLOCK:
	    DisarmVTimer(&sched->cKThread->ctrl.g->vTimer, &sched->cKThread->ctrl.g->vClock);
	    return XM_OK;
	    break;
	case XM_WATCHDOG_TIMER:
	    DisarmKTimer(sched->cKThread->ctrl.g->watchdogTimer);
	    return XM_OK;
	    break;
	default:
	    return XM_INVALID_PARAM;
	}
    }
    // Arming a timer
    switch(clockId) {
    case XM_HW_CLOCK:
	return ArmKTimer(sched->cKThread->ctrl.g->kTimer, abstime, interval);
	break;
    case XM_EXEC_CLOCK:
	return ArmVTimer(&sched->cKThread->ctrl.g->vTimer, &sched->cKThread->ctrl.g->vClock, abstime, interval);
	break;
    case XM_WATCHDOG_TIMER:
	return ArmKTimer(sched->cKThread->ctrl.g->watchdogTimer, abstime, interval);
	break;
    default:
	return XM_INVALID_PARAM;
    }

    return XM_OK;
}
 
__hypercall xm_s32_t GetTimeSys(hypercallCtxt_t *ctxt, xm_u32_t clockId, xmTime_t *__gParam time) {
    localSched_t *sched=GET_LOCAL_SCHED();  
    CHECK_SHMAGIC(sched->cKThread);
    if (__CheckGParam(ctxt, time, sizeof(xmTime_t))<0) return XM_INVALID_PARAM;
    
    if (!time)
	return XM_INVALID_PARAM;

    switch(clockId) {
    case XM_HW_CLOCK:
	*time=GetSysClockUsec();
	break;
	
    case XM_EXEC_CLOCK:
	*time=GetTimeUsecVClock(&sched->cKThread->ctrl.g->vClock);
	break;

    default:
	return XM_INVALID_PARAM;
    }

    return XM_OK;
}

__hypercall xm_s32_t MaskHwIrqSys(hypercallCtxt_t *ctxt, xm_u32_t noIrq) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);

    if (noIrq>=HWIRQ_NR) return XM_INVALID_PARAM;
    if (xmcTab.hpv.hwIrqTab[noIrq].owner!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;
  
    HwDisableIrq(noIrq);
    return XM_OK;
}

__hypercall xm_s32_t UnmaskHwIrqSys(hypercallCtxt_t *ctxt, xm_u32_t noIrq) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
  
    if (noIrq>=HWIRQ_NR) return XM_INVALID_PARAM;
    if (xmcTab.hpv.hwIrqTab[noIrq].owner!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;
    
    HwEnableIrq(noIrq);
    return XM_OK;
}

__hypercall xm_s32_t ReadObjectSys(hypercallCtxt_t *ctxt, xmObjDesc_t objDesc, void *__gParam buffer, xmSize_t size, xm_u32_t *__gParam flags) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t class;

    ASSERT(!HwIsSti());
    CHECK_SHMAGIC(sched->cKThread);
    if (__CheckGParam(ctxt, buffer, size)<0) return XM_INVALID_PARAM;
    if (__CheckGParam(ctxt, flags, sizeof(xm_u32_t))<0) return XM_INVALID_PARAM;

    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Read)
	return objectTab[class]->Read(objDesc, buffer, size, flags);
    
    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t WriteObjectSys(hypercallCtxt_t *ctxt, xmObjDesc_t objDesc, void *__gParam buffer, xmSize_t size, xm_u32_t *__gParam flags) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t class;

    ASSERT(!HwIsSti());
    CHECK_SHMAGIC(sched->cKThread);
    if (__CheckGParam(ctxt, buffer, size)<0) return XM_INVALID_PARAM;
    if (__CheckGParam(ctxt, flags, sizeof(xm_u32_t))<0) return XM_INVALID_PARAM;

    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Write)
	return objectTab[class]->Write(objDesc, buffer, size, flags);
    
    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t SeekObjectSys(hypercallCtxt_t *ctxt, xmObjDesc_t objDesc, xmAddress_t offset, xm_u32_t whence) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t class;

    ASSERT(!HwIsSti());
    CHECK_SHMAGIC(sched->cKThread);
    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Seek)
	return objectTab[class]->Seek(objDesc, offset, whence);

    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t CtrlObjectSys(hypercallCtxt_t *ctxt, xmObjDesc_t objDesc, xm_u32_t cmd, void *__gParam arg) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t class;

    ASSERT(!HwIsSti());
    CHECK_SHMAGIC(sched->cKThread);
    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Ctrl)
	return objectTab[class]->Ctrl(objDesc, cmd, arg);

    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t RaiseIPVISys(hypercallCtxt_t *ctxt, xm_u32_t partitionId, xm_u8_t noIPVI) {
    localSched_t *sched=GET_LOCAL_SCHED();
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());

    if ((partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
	return XM_INVALID_PARAM;

    if (noIPVI>=XM_MAX_IPVI) 
	return XM_INVALID_PARAM;

    SetExtIrqPending(partitionTab[partitionId], noIPVI+XM_VT_EXT_IPVI0);

    return XM_OK;
}

#ifdef CONFIG_SPARE_SCHEDULING
__hypercall xm_s32_t SetSpareGuestSys(hypercallCtxt_t *ctxt, xmId_t partitionId, xmTime_t *__gParam start, xmTime_t *__gParam stop) {
    localSched_t *sched=GET_LOCAL_SCHED();

    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SPARE_HOST_F)) {
        return XM_PERM_ERROR;
    }
    if (partitionId==sched->cKThread->ctrl.g->cfg->id)
        return XM_INVALID_PARAM;
    if ((partitionId>=xmcTab.noPartitions)||!partitionTab[partitionId])
        return XM_INVALID_PARAM;
    if (IS_KTHREAD_FLAG_SET(partitionTab[partitionId], KTHREAD_HALTED_F))
        return XM_INVALID_PARAM;

    SetSpareGuest(partitionTab[partitionId]);
    if (__CheckGParam(ctxt, start, sizeof(xmTime_t))>=0)
        *start = GetSysClockUsec();
    Scheduling();
    if (__CheckGParam(ctxt, stop, sizeof(xmTime_t))>=0)
        *stop = GetSysClockUsec();

    return XM_OK;
}
#endif
