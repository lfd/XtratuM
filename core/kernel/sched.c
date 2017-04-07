/*
 * $FILE: sched.c
 *
 * Scheduling related stuffs
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
#include <boot.h>
#include <brk.h>
#include <irqs.h>
#include <sched.h>
#include <stdc.h>
#include <arch/cswitch.h>
#include <kdevice.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif

kThread_t **partitionTab;

static kThread_t *spareGuest;
void SetSpareGuest(kThread_t *guest) {
    spareGuest = guest;
}

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

#ifdef CONFIG_SPARE_SCHEDULING
    spareGuest = NULL;
#endif
}

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
    xmTime_t cTime = GetSysClockUsec();
    localSched_t *sched = GET_LOCAL_SCHED();
    struct cyclicData *cyclicData = sched->schedulerData;
    const struct xmcSchedCyclicPlan *plan;
    kThread_t *newK = 0;
    xm_u32_t t, nextTime;
    xm_s32_t slotTabEntry;
#ifdef CONFIG_SPARE_SCHEDULING
    xm_s32_t spareSlot=0;
#endif

    plan = cyclicData->plan.current;
    if (cyclicData->mjf <= cTime) {
        MakePlanSwitch(cTime, cyclicData);
        plan = cyclicData->plan.current;
        if (cyclicData->slot < 0) {
            cyclicData->sExec = cTime;
            cyclicData->mjf = plan->majorFrame + cyclicData->sExec;
        } else {
            cyclicData->sExec = cyclicData->mjf;
            cyclicData->mjf += plan->majorFrame;
#ifdef CONFIG_OBJ_STATUS_ACC
            systemStatus.currentMaf++;
#endif
        }
        cyclicData->slot = 0;
        cyclicData->newSlot = 1;
#ifdef CONFIG_SPARE_SCHEDULING
        spareGuest = NULL;
#endif
        CLEAR_KTHREAD_FLAG(partitionTab[xmcSchedCyclicSlotTab[plan->slotsOffset+(plan->noSlots-1)].partitionId], KTHREAD_YIELD_F);
    }
    t = cTime - cyclicData->sExec;
    nextTime = plan->majorFrame;

    // Calculate our next slot
    if (cyclicData->slot >= plan->noSlots)
        goto out;
    // getting idle

    while (t >= xmcSchedCyclicSlotTab[plan->slotsOffset + cyclicData->slot].eExec) {
        CLEAR_KTHREAD_FLAG(partitionTab[xmcSchedCyclicSlotTab[plan->slotsOffset+cyclicData->slot].partitionId], KTHREAD_YIELD_F);
        cyclicData->slot++;
        cyclicData->newSlot = 1;
        if (cyclicData->slot >= plan->noSlots)
            goto out;
        // getting idle
    }
    slotTabEntry = plan->slotsOffset + cyclicData->slot;

    if (t >= xmcSchedCyclicSlotTab[slotTabEntry].sExec) {
        ASSERT((xmcSchedCyclicSlotTab[slotTabEntry].partitionId>=0)&&(xmcSchedCyclicSlotTab[slotTabEntry].partitionId<xmcTab.noPartitions));
        ASSERT(partitionTab[xmcSchedCyclicSlotTab[slotTabEntry].partitionId]);
        newK = partitionTab[xmcSchedCyclicSlotTab[slotTabEntry].partitionId];
        if (!IS_KTHREAD_FLAG_SET(newK, KTHREAD_HALTED_F) && !IS_KTHREAD_FLAG_SET(newK, KTHREAD_YIELD_F) && IS_KTHREAD_FLAG_SET(newK, KTHREAD_READY_F)) {
            nextTime = xmcSchedCyclicSlotTab[slotTabEntry].eExec;
        } else {
            newK = 0;
            if ((cyclicData->slot + 1) < plan->noSlots)
                nextTime = xmcSchedCyclicSlotTab[slotTabEntry + 1].sExec;
        }
    } else {
        nextTime = xmcSchedCyclicSlotTab[slotTabEntry].sExec;
    }

#ifdef CONFIG_SPARE_SCHEDULING
    if (newK && IS_KTHREAD_FLAG_SET(newK, KTHREAD_SPARE_HOST_F)) {
        if (!cyclicData->newSlot) {
            if (spareGuest && !IS_KTHREAD_FLAG_SET(spareGuest, KTHREAD_HALTED_F)) {
                newK = spareGuest;
                spareSlot = 1;
            }
        }
    }
#endif

out:
    ArmKTimer(cyclicData->kTimer, nextTime + cyclicData->sExec, 0);
    slotTabEntry = plan->slotsOffset + cyclicData->slot;

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
        if (newK && newK->ctrl.g) {
            cyclicData->newSlot = 0;
            newK->ctrl.g->partitionControlTable->schedInfo.noSlot = cyclicData->slot;
#ifdef CONFIG_SPARE_SCHEDULING
            if (spareSlot)
                newK->ctrl.g->partitionControlTable->schedInfo.reserved = XM_PCT_SLOT_SPARE;
            else
                newK->ctrl.g->partitionControlTable->schedInfo.reserved = XM_PCT_SLOT_CYCLIC;
#endif
            newK->ctrl.g->partitionControlTable->schedInfo.id = xmcSchedCyclicSlotTab[slotTabEntry].id;
            newK->ctrl.g->partitionControlTable->schedInfo.slotDuration = xmcSchedCyclicSlotTab[slotTabEntry].eExec - xmcSchedCyclicSlotTab[slotTabEntry].sExec;
            SetExtIrqPending(newK, XM_VT_EXT_CYCLIC_SLOT_START);
        }
    }
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
    localSched_t *sched = GET_LOCAL_SCHED();
    localCpu_t *cpu=GET_LOCAL_CPU();
    xm_u32_t hwFlags;
    kThread_t *newK;

    CHECK_KTHR_SANITY(sched->cKThread);
    if (!(sched->flags&LOCAL_SCHED_ENABLED)) {
        cpu->irqNestingCounter&=~(SCHED_PENDING);
        return;
    }

    HwSaveFlagsCli(hwFlags);

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
#ifdef CONFIG_DEV_WB_GPIO
#define CONFIG_VOS4ES_DEMO
#ifdef CONFIG_VOS4ES_DEMO
        static xm_u8_t val=0x0;
        static xm_u8_t vec[] = {0, 1, 6, 7};
        extern const kDevice_t gpioDev;
        val = 0;
        if (newK->ctrl.g) {
            val = 1<<vec[newK->ctrl.g->cfg->id];
        }
        gpioDev.Write(NULL, &val, 1);
#endif
#endif

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

