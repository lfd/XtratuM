/*
 * $FILE: status.c
 *
 * Status functionality
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
#include <brk.h>
#include <boot.h>
#include <hypercalls.h>
#include <kthread.h>
#include <stdc.h>
#include <sched.h>
#include <objects/status.h>

xmSystemStatus_t systemStatus;
xmPartitionStatus_t *partitionStatus;

static xm_s32_t CtrlStatus(xmObjDesc_t desc, xm_u32_t cmd, union statusCmd *__gParam args) {
    localSched_t *sched=GET_LOCAL_SCHED();
    extern xm_u32_t sysResetCounter[];
    xmId_t partId;
    
    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;

    if (!args)
        return XM_INVALID_PARAM;
    if (__CheckGParam(0, args, sizeof(union statusCmd))<0) 
	return XM_INVALID_PARAM;
    switch(cmd) {
    case XM_STATUS_GET:
	if (partId==XM_HYPERVISOR_ID) {
	    systemStatus.resetCounter=sysResetCounter[0];
	    memcpy(&args->status.system, &systemStatus, sizeof(xmSystemStatus_t));
	} else {
	    if ((partId<0)||(partId>=xmcTab.noPartitions))
		return XM_INVALID_PARAM;
	    if (IS_KTHREAD_FLAG_SET(partitionTab[partId], KTHREAD_HALTED_F)) {
		partitionStatus[partId].state=XM_STATUS_HALTED;
	    } else if (IS_KTHREAD_FLAG_SET(partitionTab[partId], KTHREAD_YIELD_F)) {
		partitionStatus[partId].state=XM_STATUS_IDLE;
	    } else if (IS_KTHREAD_FLAG_SET(partitionTab[partId], KTHREAD_READY_F)) {
		partitionStatus[partId].state=XM_STATUS_READY;
	    } else {
		partitionStatus[partId].state=XM_STATUS_SUSPENDED;
	    }
	    partitionStatus[partId].resetCounter=partitionTab[partId]->ctrl.g->resetCounter;
	    partitionStatus[partId].resetStatus=partitionTab[partId]->ctrl.g->resetStatus;
	    partitionStatus[partId].execClock=GetTimeUsecVClock(&partitionTab[partId]->ctrl.g->vClock);
	    memcpy(&args->status.partition, &partitionStatus[partId].state, sizeof(xmPartitionStatus_t));

	}
	
	return XM_OK;
	break;

    case XM_GET_SCHED_PLAN_STATUS:
        args->status.plan.switchTime = sched->schedulerData->planSwitchTime;
        args->status.plan.next = (sched->schedulerData->plan.new ? sched->schedulerData->plan.new->id : -1);
        args->status.plan.current = (sched->schedulerData->plan.current ? sched->schedulerData->plan.current->id : -1);
        args->status.plan.prev = (sched->schedulerData->plan.prev ? sched->schedulerData->plan.prev->id : -1);
        return XM_OK;
    case XM_SWITCH_SCHED_PLAN:
        if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
            return XM_PERM_ERROR;

        return (SwitchSchedPlan(args->schedPlan.new, &args->schedPlan.current)==0)?XM_OK:XM_INVALID_PARAM;
    }
    return XM_INVALID_PARAM;
}

static const struct object statusObj={
    .Ctrl=(ctrlObjOp_t)CtrlStatus,
};

xm_s32_t __VBOOT SetupStatus(void) {
    GET_MEMZ(partitionStatus, sizeof(xmPartitionStatus_t)*xmcTab.noPartitions);
    objectTab[OBJ_CLASS_STATUS]=&statusObj;

    return 0;
}

REGISTER_OBJ(SetupStatus);


