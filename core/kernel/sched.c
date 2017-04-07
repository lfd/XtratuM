/*
 * $FILE: sched.c
 *
 * Scheduling related stuffs
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
#include <brk.h>
#include <irqs.h>
#include <sched.h>
#include <stdc.h>
#include <arch/cswitch.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif
/*#ifdef CONFIG_SPARE_SCHEDULING*/
#include <objects/spare.h>
/*#endif*/

kThread_t **partitionTab;

void __VBOOT InitSched(void) {
    GET_MEMZ(partitionTab, sizeof(kThread_t *)*xmcTab.noPartitions);
}

void __VBOOT InitSchedLocal(kThread_t *idle, xm_s32_t schedId, const union xmcSchedParams *schedParams) {
    localSched_t *sched=GET_LOCAL_SCHED();

    ASSERT((schedId<SCHED_NR)&&schedList[schedId]);

    InitIdle(idle, GET_CPU_ID());
    sched->cKThread=sched->idleKThread=idle;
    sched->scheduler=schedList[schedId];

    kprintf("[sched] using %s scheduler\n", sched->scheduler->name);
  
    if (sched->scheduler->Init)
	sched->scheduler->Init(idle, schedParams);

}

#ifdef CONFIG_SPARE_SCHEDULING

/////////////////// SPARE SCHEDULER

struct spareData {
    xm_u16_t slot;              // Next slot to be processed
#define _ST_SPARE_NEW_SLOT 0x1  // New slot scheduled
#define _ST_SPARE_LAUNCHED 0x2  // Spare plan launched and running
#define _ST_SPARE_SPLIT 0x4     // Spare slot has been split
    xm_u16_t status;            // Spare plan status
    xmTime_t remaining;         // Remaining slot time
    xm_s32_t current;
    xm_s32_t new;
};
static struct spareData spareData[CONFIG_NO_CPUS];
static struct xmcSchedSparePlan xmcSchedSparePlan[CONFIG_NO_CPUS][2];
static struct xmcSchedSpareSlot xmcSchedSpareSlots[CONFIG_NO_CPUS][2][CONFIG_MAX_SPARE_SLOTS];

static void InitSpare(void) {
    memset(&spareData[GET_CPU_ID()], 0, sizeof(spareData));
    memset(xmcSchedSparePlan, 0, sizeof(xmcSchedSparePlan));
    xmcSchedSparePlan[GET_CPU_ID()][0].slotTab = xmcSchedSpareSlots[GET_CPU_ID()][0];
    xmcSchedSparePlan[GET_CPU_ID()][1].slotTab = xmcSchedSpareSlots[GET_CPU_ID()][1];
    spareData->slot = -1;
    spareData->new = -1;
}

struct xmcSchedSparePlan *GetCurrentSparePlan(void) {
    return &xmcSchedSparePlan[GET_CPU_ID()][spareData->current];
}

xm_s32_t FillSparePartitionTab(struct xmcSchedSparePlan *sparePlan) {
    xm_s32_t e, k=0;

    sparePlan->header.cycleTime = 0;
    for (e=0; e<xmcTab.noPartitions; e++) {
        if (partitionTab[e]->ctrl.flags & KTHREAD_SPARE_GUEST_F) {
            sparePlan->slotTab[k++].partitionId = e;
        }
        if (k >= CONFIG_MAX_SPARE_SLOTS) {
            return -1;
        }
    }
    sparePlan->slotTab[k].partitionId = -1;
    sparePlan->header.noSlots = CONFIG_MAX_SPARE_SLOTS;
    return 0;
}

// This function has to check the entire spare plan for inconsistencies. It
// may take some time to execute, depending on the configured maximum number
// of spare slots. Host is not allowed to be scheduled on the spare plan
xm_s32_t SetSparePlan(struct xmcSchedSparePlan *newSparePlan) {
    xm_s32_t e, acc=0;

    if (!newSparePlan) {
        spareData->new = -1;
        return 0;
    }
    if (newSparePlan->header.noSlots > CONFIG_MAX_SPARE_SLOTS || newSparePlan->header.noSlots == 0)
        return -1;
    for (e=0; e<newSparePlan->header.noSlots; e++) {
        if ((!((newSparePlan->slotTab[e].partitionId>=0)&&(newSparePlan->slotTab[e].partitionId<xmcTab.noPartitions))) ||
            (!IS_KTHREAD_FLAG_SET(partitionTab[newSparePlan->slotTab[e].partitionId], KTHREAD_SPARE_GUEST_F)) ||
            (IS_KTHREAD_FLAG_SET(partitionTab[newSparePlan->slotTab[e].partitionId], KTHREAD_SPARE_HOST_F)) ||
            ((acc + newSparePlan->slotTab[e].duration) > newSparePlan->header.cycleTime))
            return -1;
        acc += newSparePlan->slotTab[e].duration;
    }
    memcpy(&xmcSchedSparePlan[GET_CPU_ID()][1-spareData->current],
           newSparePlan,
           sizeof(struct xmcSchedSparePlanHeader));
    memcpy(xmcSchedSparePlan[GET_CPU_ID()][1-spareData->current].slotTab,
           newSparePlan->slotTab,
           newSparePlan->header.noSlots*sizeof(struct xmcSchedSpareSlot));
    spareData->new = 1 - spareData->current;
    return 0;
}

static kThread_t *GetReadyKThreadSpare(kThread_t *sThread, xmTime_t *nextTime) {
    kThread_t *newK=0;
    const struct xmcSchedSparePlan *plan;
    localSched_t *sched=GET_LOCAL_SCHED();

    *nextTime = 0;
    if (spareData->new < 0) { // No plan to run
        if (IS_KTHREAD_FLAG_SET(sThread, KTHREAD_YIELD_F)) {
            return 0;
        } else {
            return sThread;
        }
    }
    plan = &xmcSchedSparePlan[GET_CPU_ID()][spareData->current];

    // Don't miss slots
    spareData->status|=_ST_SPARE_NEW_SLOT;
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SPARE_HOST_F)) {
        if (spareData->remaining <= 0) {
            spareData->slot++;
        } else if (IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_YIELD_F)) {
            CLEAR_KTHREAD_FLAG(sched->cKThread, KTHREAD_YIELD_F);
            spareData->slot++;
        } else {
            spareData->status&=~_ST_SPARE_NEW_SLOT;
        }
        if (spareData->slot >= plan->header.noSlots) {
            return sThread;
        }
    } else { // Launch the plan
        if (IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_YIELD_F)) {
            spareData->slot=0;
            spareData->current = spareData->new;
            plan = &xmcSchedSparePlan[GET_CPU_ID()][spareData->current];
        }
    }

    newK=partitionTab[plan->slotTab[spareData->slot].partitionId];
    while (IS_KTHREAD_FLAG_SET(newK, KTHREAD_HALTED_F)) {
        spareData->slot++;
        if (spareData->slot >= plan->header.noSlots) {
            return sThread;
        }
        spareData->status|=_ST_SPARE_NEW_SLOT;
        newK=partitionTab[plan->slotTab[spareData->slot].partitionId];
    }
    CLEAR_KTHREAD_FLAG(newK, KTHREAD_YIELD_F);
    if (spareData->status&_ST_SPARE_NEW_SLOT) {
        spareData->remaining = plan->slotTab[spareData->slot].duration;
    }
    spareData->status|=_ST_SPARE_NEW_SLOT;
    *nextTime = spareData->remaining;

    return newK;
}
#endif

////////////////// CYCLIC SCHEDULER

static struct cyclicData cyclicData[CONFIG_NO_CPUS];
static xm_s32_t InitCyclic(kThread_t *idle, const union xmcSchedParams *schedParams) {
    localSched_t *sched=GET_LOCAL_SCHED();

    //CheckCyclicPlan(&schedParams->cyclic);
    memset(&cyclicData[GET_CPU_ID()], 0, sizeof(struct cyclicData));
    sched->schedulerData=&cyclicData[GET_CPU_ID()];
    cyclicData->kTimer=AllocKTimer((void (*)(struct kTimer *, void *))SetSchedPending, NULL, NULL);
    cyclicData->cfgTab=&schedParams->cyclic;
    cyclicData->slot=-1;
    cyclicData->plan.current=0;
    cyclicData->plan.prev=0;
    cyclicData->plan.new=&xmcSchedCyclicPlanTab[xmcTab.hpv.cpuTab[GET_CPU_ID()].schedParams.cyclic.schedCyclicPlansOffset];

#ifdef CONFIG_SPARE_SCHEDULING
    InitSpare();
#endif

    return 0;
}

xm_s32_t SwitchSchedPlan(xm_s32_t newPlanId, xm_s32_t *oldPlanId) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct cyclicData *cyclicData=sched->schedulerData;

    if ((newPlanId<1)||(newPlanId>=xmcTab.hpv.cpuTab[GET_CPU_ID()].schedParams.cyclic.noSchedCyclicPlans))
        return -1;

    *oldPlanId=-1;
    if (cyclicData->plan.current)
        *oldPlanId=cyclicData->plan.current->id;

    cyclicData->plan.new=&xmcSchedCyclicPlanTab[xmcTab.hpv.cpuTab[GET_CPU_ID()].schedParams.cyclic.schedCyclicPlansOffset+newPlanId];
    return 0;
}

inline void MakePlanSwitch(xmTime_t cTime, struct cyclicData *cyclic) {
    if (cyclic->plan.current!=cyclic->plan.new) {
        cyclic->plan.prev=cyclic->plan.current;
        cyclic->plan.current=cyclic->plan.new;
        cyclic->planSwitchTime=cTime;
        cyclic->slot=-1;
        cyclic->mjf=0;
    }
}

static kThread_t *GetReadyKThreadCyclic(kThread_t *cKThread, kThread_t *idle, void *data) {
    xmTime_t cTime=GetSysClockUsec();    
    localSched_t *sched=GET_LOCAL_SCHED();
    struct cyclicData *cyclicData=sched->schedulerData;
    const struct xmcSchedCyclicPlan *plan;
    kThread_t *newK=0;
    xm_u32_t t, nextTime;
    xm_s32_t slotTabEntry;
#ifdef CONFIG_SPARE_SCHEDULING
    xmTime_t spareNextTime;
#endif

    plan=cyclicData->plan.current;
    if (cyclicData->mjf<=cTime) {
        MakePlanSwitch(cTime, cyclicData);
        plan=cyclicData->plan.current;
	if (cyclicData->slot<0) {
	    cyclicData->sExec=cTime;
	    cyclicData->mjf=plan->majorFrame+cyclicData->sExec;
	} else {
	    cyclicData->sExec=cyclicData->mjf;
	    cyclicData->mjf+=plan->majorFrame;
#ifdef CONFIG_OBJ_STATUS_ACC
	    systemStatus.currentMaf++;
#endif
	}

	cyclicData->slot=0;
	cyclicData->newSlot=1;
	CLEAR_KTHREAD_FLAG(partitionTab[xmcSchedCyclicSlotTab[plan->slotsOffset+(plan->noSlots-1)].partitionId], KTHREAD_YIELD_F);
    }
    t=cTime-cyclicData->sExec;
    nextTime=plan->majorFrame;

    // Calculate our next slot
    if (cyclicData->slot>=plan->noSlots)
	goto out; // getting idle
    
    while (t>=xmcSchedCyclicSlotTab[plan->slotsOffset+cyclicData->slot].eExec) {
	CLEAR_KTHREAD_FLAG(partitionTab[xmcSchedCyclicSlotTab[plan->slotsOffset+cyclicData->slot].partitionId], KTHREAD_YIELD_F);
	cyclicData->slot++;
	cyclicData->newSlot=1;
	if (cyclicData->slot>=plan->noSlots)
	    goto out; // getting idle
    }
    slotTabEntry=plan->slotsOffset+cyclicData->slot;

    if (t>=xmcSchedCyclicSlotTab[slotTabEntry].sExec) {
	ASSERT((xmcSchedCyclicSlotTab[slotTabEntry].partitionId>=0)&&(xmcSchedCyclicSlotTab[slotTabEntry].partitionId<xmcTab.noPartitions));
	ASSERT(partitionTab[xmcSchedCyclicSlotTab[slotTabEntry].partitionId]);
	newK=partitionTab[xmcSchedCyclicSlotTab[slotTabEntry].partitionId];
	if (!IS_KTHREAD_FLAG_SET(newK, KTHREAD_HALTED_F)&&!IS_KTHREAD_FLAG_SET(newK, KTHREAD_YIELD_F)&&IS_KTHREAD_FLAG_SET(newK, KTHREAD_READY_F)) {
	    nextTime=xmcSchedCyclicSlotTab[slotTabEntry].eExec;
	} else {
#ifdef CONFIG_SPARE_SCHEDULING
        if (!(IS_KTHREAD_FLAG_SET(newK, KTHREAD_SPARE_HOST_F)))/* && (spareData->status&_ST_SPARE_LAUNCHED)))*/
#endif
	    newK=0;
	    if ((cyclicData->slot+1)<plan->noSlots)
		nextTime=xmcSchedCyclicSlotTab[slotTabEntry+1].sExec;
	}
    } else {
	nextTime=xmcSchedCyclicSlotTab[slotTabEntry].sExec;
    }

#ifdef CONFIG_SPARE_SCHEDULING
#define XM_SPARE_DELTA 50 //useconds
    if (newK && IS_KTHREAD_FLAG_SET(newK, KTHREAD_SPARE_HOST_F)) {
        if ((nextTime + cyclicData->sExec - cTime) > XM_SPARE_DELTA) { // Spare run allowed
            newK = GetReadyKThreadSpare(newK, &spareNextTime);
            if (spareNextTime > 0) {
                if ((cTime + spareNextTime) < (nextTime + cyclicData->sExec)) {
                    nextTime = (cTime + spareNextTime) - cyclicData->sExec;
                    spareData->remaining = 0;
                } else {
                    spareData->remaining -= nextTime + cyclicData->sExec - cTime;
                }
            }
        } else {
            newK = 0;
        }
    }
#endif

out:
    ArmKTimer(cyclicData->kTimer, nextTime+cyclicData->sExec, 0);
    slotTabEntry=plan->slotsOffset+cyclicData->slot;

#if 0
    if (newK) {
	kprintf("[%d:%d] cTime %lld -> sExec %lld eExec %lld\n",
		cyclicData->slot,
		xmcSchedCyclicSlotTab[slotTabEntry].partitionId,
		cTime, xmcSchedCyclicSlotTab[slotTabEntry].sExec+cyclicData->sExec,
		xmcSchedCyclicSlotTab[slotTabEntry].eExec+cyclicData->sExec);
    } else {
	kprintf("IDLE: %lld\n", cTime);
    }
#endif
  
    if (cyclicData->newSlot) {
	if (newK&&newK->ctrl.g) {
	    cyclicData->newSlot=0;
	    newK->ctrl.g->partitionControlTable->schedInfo.noSlot=cyclicData->slot;
	    newK->ctrl.g->partitionControlTable->schedInfo.reserved = XM_PCT_SLOT_CYCLIC;
	    newK->ctrl.g->partitionControlTable->schedInfo.id=xmcSchedCyclicSlotTab[slotTabEntry].id;
	    newK->ctrl.g->partitionControlTable->schedInfo.slotDuration=xmcSchedCyclicSlotTab[slotTabEntry].eExec-xmcSchedCyclicSlotTab[slotTabEntry].sExec;
	    SetExtIrqPending(newK, XM_VT_EXT_CYCLIC_SLOT_START);
	}
    }
#ifdef CONFIG_SPARE_SCHEDULING
    if (spareData->status&_ST_SPARE_NEW_SLOT) {
        if (newK&&newK->ctrl.g) {
            struct xmcSchedSpareSlot *cSlot = &xmcSchedSpareSlots[GET_CPU_ID()][spareData->current][spareData->slot];
            spareData->status&=~_ST_SPARE_NEW_SLOT;
            newK->ctrl.g->partitionControlTable->schedInfo.noSlot=spareData->slot;
            newK->ctrl.g->partitionControlTable->schedInfo.reserved = XM_PCT_SLOT_SPARE;
            newK->ctrl.g->partitionControlTable->schedInfo.id=cSlot->id;
            newK->ctrl.g->partitionControlTable->schedInfo.slotDuration=nextTime + cyclicData->sExec - cTime;
            SetExtIrqPending(newK, XM_VT_EXT_CYCLIC_SLOT_START);
        }
    }
#endif
    return newK;
} 

static void AddKThreadCyclic(kThread_t *k) {
}

static void ShutdownCyclic(void) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct cyclicData *cyclicData=sched->schedulerData;

    FreeKTimer(cyclicData->kTimer, NULL);
    sched->schedulerData=0;
}

static struct scheduler cyclic_sched={
    .name="cyclic",
    .Init=InitCyclic,
    .GetReadyKThread=GetReadyKThreadCyclic,
    .AddKThread=AddKThreadCyclic,
    .Shutdown=ShutdownCyclic,
};

/////////////////////////////////////////////////////////

void SetSchedPending(void) {
    localCpu_t *cpu=GET_LOCAL_CPU();
    cpu->irqNestingCounter|=SCHED_PENDING;
}

void Scheduling(void) {
    localSched_t *sched=GET_LOCAL_SCHED();
    localCpu_t *cpu=GET_LOCAL_CPU();
    xm_u32_t hwFlags;
    kThread_t *newK;

    CHECK_KTHR_SANITY(sched->cKThread);
    if (!(sched->flags&LOCAL_SCHED_ENABLED)) {
	cpu->irqNestingCounter&=~(SCHED_PENDING);
	return;
    }

    HwSaveFlagsCli(hwFlags);
    // When an interrupt is in-progress, the scheduler shouldn't be invoked
    if (cpu->irqNestingCounter&IRQ_IN_PROGRESS) {
	cpu->irqNestingCounter|=SCHED_PENDING;
	HwRestoreFlags(hwFlags);
	return;
    }
    cpu->irqNestingCounter&=(~SCHED_PENDING);
    if (!sched->scheduler->GetReadyKThread||!(newK=sched->scheduler->GetReadyKThread(sched->cKThread, sched->idleKThread, sched->schedulerData)))
	newK=sched->idleKThread;
    CHECK_KTHR_SANITY(newK);

    if (newK!=sched->cKThread) {
#if 0
	if (newK->ctrl.g)
	    kprintf("newK: %d 0x%x ", newK->ctrl.g->cfg->id, newK);
	else
	    kprintf("newK: idle ");

	if (sched->cKThread->ctrl.g)
	    kprintf("curK: %d 0x%x\n", sched->cKThread->ctrl.g->cfg->id, sched->cKThread);
	else
	    kprintf("curK: idle\n");
#endif
	SwitchKThreadArchPre(newK, sched->cKThread);
	if (sched->cKThread->ctrl.g)
	    StopVClock(&sched->cKThread->ctrl.g->vClock,  &sched->cKThread->ctrl.g->vTimer);
	CONTEXT_SWITCH(newK, &sched->cKThread);

	if (sched->cKThread->ctrl.g)
	    ResumeVClock(&sched->cKThread->ctrl.g->vClock, &sched->cKThread->ctrl.g->vTimer);
	SwitchKThreadArchPost(sched->cKThread);
    }
    HwRestoreFlags(hwFlags);
}

xm_s32_t AddKThread(kThread_t *k, xm_s32_t cpu) {
    ASSERT(cpu<GET_NRCPUS());
    if (localSchedInfo[cpu].scheduler->AddKThread)
	localSchedInfo[cpu].scheduler->AddKThread(k);
    return 0;
}

struct scheduler *schedList[SCHED_NR]={&cyclic_sched,};

