/*
 * $FILE: hm.c
 *
 * Health Monitor
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */


#include <assert.h>
#include <boot.h>
#include <kthread.h>
#include <hypercalls.h>
#include <logstream.h>
#include <brk.h>
#include <stdc.h>
#include <xmconf.h>
#include <sched.h>
#include <objects/hm.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif

static struct logStream hmLogStream;

static xm_s32_t ReadHmLog(xmObjDesc_t desc, xmHmLog_t *log, xm_u32_t size) {
    xm_s32_t e, noLogs=size/sizeof(xmHmLog_t);
    localSched_t *sched=GET_LOCAL_SCHED();

    if (OBJDESC_GET_PARTITIONID(desc)!=XM_HYPERVISOR_ID)
	return XM_INVALID_PARAM;

    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
        return XM_PERM_ERROR;

    if (!log||!noLogs)
        return XM_INVALID_PARAM;
    if (__CheckGParam(0, log, sizeof(xmHmLog_t)*noLogs)<0)
        return XM_INVALID_PARAM;

    for (e=0; e<noLogs; e++)
	if (LogStreamGet(&hmLogStream, &log[e])<0)
	    return e*sizeof(xmHmLog_t);
    
    return noLogs*sizeof(xmHmLog_t);
}

static xm_s32_t SeekHmLog(xmObjDesc_t desc, xm_u32_t offset, xm_u32_t whence) {
    localSched_t *sched=GET_LOCAL_SCHED();
    if (OBJDESC_GET_PARTITIONID(desc)!=XM_HYPERVISOR_ID)
	return XM_INVALID_PARAM;
    
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
        return XM_PERM_ERROR;

    return LogStreamSeek(&hmLogStream, offset, whence);
}

static xm_s32_t CtrlHmLog(xmObjDesc_t desc, xm_u32_t cmd, union hmCmd *__gParam args) {
    localSched_t *sched=GET_LOCAL_SCHED();
    if (OBJDESC_GET_PARTITIONID(desc)!=XM_HYPERVISOR_ID)
	return XM_INVALID_PARAM;
    
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
        return XM_PERM_ERROR;
    if (!args)
        return XM_INVALID_PARAM;
    if (__CheckGParam(0, args, sizeof(union hmCmd))<0)
        return XM_INVALID_PARAM;
    switch(cmd) {
    case XM_HM_GET_STATUS:
	args->status.noEvents=hmLogStream.elem;
	args->status.maxEvents=hmLogStream.maxNoElem;
	args->status.currentEvent=hmLogStream.d;
	return XM_OK;
	break;
    }
    return XM_INVALID_PARAM;
}

static const struct object hmObj={
    .Read=(readObjOp_t)ReadHmLog,
    .Seek=(seekObjOp_t)SeekHmLog,
    .Ctrl=(ctrlObjOp_t)CtrlHmLog,
};

xm_s32_t __VBOOT SetupHm(void) {
    LogStreamInit(&hmLogStream, LookUpKDev(&xmcTab.hpv.hmDev), sizeof(xmHmLog_t));
    objectTab[OBJ_CLASS_HM]=&hmObj;
    return 0;
}

REGISTER_OBJ(SetupHm);

xm_s32_t HmRaiseEvent(xmHmLog_t *log) {
    xm_s32_t propagate=0;
    ASSERT((log->eventId>=0)&&(log->eventId<XM_HM_MAX_EVENTS));
#ifdef CONFIG_OBJ_STATUS_ACC
    systemStatus.noHmEvents++;
#endif
    log->timeStamp=GetSysClockUsec();
    kprintf("[HM:%lld] event %d: sys %d: Id %d\n", log->timeStamp, log->eventId, log->system, log->partitionId);
    kprintf("0x%x 0x%x 0x%x\n", log->word[0], log->word[1], log->word[2]);
    kprintf("0x%x 0x%x\n", log->word[3], log->word[4]);
    if (log->system) {
	if (xmcTab.hpv.hmTab[log->eventId].log)
	    LogStreamInsert(&hmLogStream, log);
	switch(xmcTab.hpv.hmTab[log->eventId].action) {
	case XM_HM_AC_IGNORE:
	    // Doing nothing
	    break;
	case XM_HM_AC_COLD_RESET:
	    HaltSystem();
	    break;
	case XM_HM_AC_WARM_RESET:
	    HaltSystem();
	    break;
	case XM_HM_AC_HALT:
	    HaltSystem();
	    break;
	default:
	    SystemPanic(0, 0, "Unknown health-monitor action %d\n", xmcTab.hpv.hmTab[log->eventId].action);
	}
    } else {
	if (partitionTab[log->partitionId]->ctrl.g->cfg->hmTab[log->eventId].log)
	    LogStreamInsert(&hmLogStream, log);

	ASSERT((log->partitionId>=0)&&(log->partitionId<xmcTab.noPartitions));
	ASSERT(partitionTab[log->partitionId]);
	switch(partitionTab[log->partitionId]->ctrl.g->cfg->hmTab[log->eventId].action) {
	case XM_HM_AC_IGNORE:
	    // Doing nothing
	    break;
	case XM_HM_AC_SHUTDOWN:
	    SetExtIrqPending(partitionTab[log->partitionId], XM_VT_EXT_SHUTDOWN);
	    break;
	case XM_HM_AC_COLD_RESET:
	    kprintf("[HM] Partition %d cold reseted\n", log->partitionId);
	    ResetPartition(partitionTab[log->partitionId], 1, log->eventId);
	    break;
	case XM_HM_AC_WARM_RESET:
	    kprintf("[HM] Partition %d warm reseted\n", log->partitionId);
	    ResetPartition(partitionTab[log->partitionId], 0, log->eventId);
	    break;
	case XM_HM_AC_SUSPEND:
	    ASSERT(log->partitionId!=XM_HYPERVISOR_ID);
	    kprintf("[HM] Partition %d suspended\n", log->partitionId);
	    CLEAR_KTHREAD_FLAG(partitionTab[log->partitionId], KTHREAD_READY_F);
	    Scheduling();
	    break;
	case XM_HM_AC_HALT:
	    ASSERT(log->partitionId!=XM_HYPERVISOR_ID);
	    kprintf("[HM] Partition %d halted\n", log->partitionId);
	    SET_KTHREAD_FLAG(partitionTab[log->partitionId], KTHREAD_HALTED_F);
	    Scheduling();
	    break;
	case XM_HM_AC_PROPAGATE:
	    propagate=1;
	    break;
	default:
	    SystemPanic(0, 0, "Unknown health-monitor action %d\n", partitionTab[log->partitionId]->ctrl.g->cfg->hmTab[log->eventId].action);
	}
    }

    return propagate;
}
