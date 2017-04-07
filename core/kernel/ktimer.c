/*
 * $FILE: ktimers.c
 *
 * XM's timer interface
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
#include <brk.h>
#include <boot.h>
#include <ktimer.h>
#include <sched.h>
#include <stdc.h>

static xm_s32_t TimerHandler(void);

inline void SetHwTimer(xmTime_t nextAct) {
    localTime_t *localTime=GET_LOCAL_TIME();
    xmTime_t nextTime, cTime;

    ASSERT(!HwIsSti());
    ASSERT(nextAct>=0);
    if (!nextAct) return;
    if ((localTime->flags&NEXT_ACTIVATION_VALID)&&(nextAct>=localTime->nextAct))
        return;
    localTime->flags|=NEXT_ACTIVATION_VALID;
    localTime->nextAct=nextAct;
    cTime=GetSysClockUsec();
    nextTime=nextAct-cTime;

    if (nextTime>=0) {
        //ASSERT(nextTime>0);
        if (nextTime<sysHwTimer->GetMinInterval())
            nextTime=sysHwTimer->GetMinInterval();

        if (nextTime>sysHwTimer->GetMaxInterval())
            nextTime=sysHwTimer->GetMaxInterval();
        localTime->nextAct=nextTime+cTime;
        sysHwTimer->SetHwTimer(nextTime);
    } else
        TimerHandler();
}

xmTime_t TraverseKTimerQueue(struct dynList *l, xmTime_t cTime) {
    xmTime_t nextAct=0x7FFFFFFFFFFFFFFFULL;
    kTimer_t *kTimer;

    DYNLIST_FOR_EACH_ELEMENT_BEGIN(l, kTimer, 1) {
        ASSERT(kTimer);
        if (kTimer->flags&KTIMER_ARMED){
            if (kTimer->value<=cTime) {
                    if (kTimer->Action)
                kTimer->Action(kTimer, kTimer->actionArgs);

            if (kTimer->interval>0) {
                // To be optimised
                do {
                    kTimer->value+=kTimer->interval;
                } while(kTimer->value<=cTime);
                if (nextAct>kTimer->value)
                    nextAct=kTimer->value;
            } else
                kTimer->flags&=~KTIMER_ARMED;
            } else {
            if (nextAct>kTimer->value)
                nextAct=kTimer->value;
            }
        }
    } DYNLIST_FOR_EACH_ELEMENT_END(l);

    return nextAct;
}

static xm_s32_t TimerHandler(void) {
    localTime_t *localTime=GET_LOCAL_TIME();
    localSched_t *sched=GET_LOCAL_SCHED();
    xmTime_t cTime, nextAct, nLocalAct;

    ASSERT(!HwIsSti());
    localTime->flags&=~NEXT_ACTIVATION_VALID;
    cTime=GetSysClockUsec();
    nextAct=TraverseKTimerQueue(&localTime->globalActiveKTimers, cTime);
    if (sched->cKThread->ctrl.g)
        if ((nLocalAct=TraverseKTimerQueue(&sched->cKThread->ctrl.localActiveKTimers, cTime))&& (nLocalAct<nextAct))
            nextAct=nLocalAct;
    SetHwTimer(nextAct);

    return 0;
}

kTimer_t *AllocKTimer(void (*Act)(kTimer_t *, void *), void *args, void *kThread) {
    localTime_t *localTime=GET_LOCAL_TIME();
    kThread_t *k=(kThread_t *)kThread;
    kTimer_t *kTimer;
    
    if (!(kTimer=(kTimer_t *)DynListRemoveTail(&localTime->freeKTimers)))
	return NULL;
    memset((char *)kTimer, 0, sizeof(kTimer_t));
    kTimer->actionArgs=args;
    kTimer->Action=Act;
    if(DynListInsertHead((k)?&k->ctrl.localActiveKTimers:&localTime->globalActiveKTimers, &kTimer->dynListPtrs))
	SystemPanic(0, 0, "[KTIMER] Error allocating ktimer");

    return kTimer;
}

void FreeKTimer(kTimer_t *kTimer, void *kThread) {
    localTime_t *localTime=GET_LOCAL_TIME();
    kThread_t *k=(kThread_t *)kThread;

    kTimer->flags=0;
    if (DynListRemoveElement((k)?&k->ctrl.localActiveKTimers:&localTime->globalActiveKTimers, &kTimer->dynListPtrs))
      SystemPanic(0, 0, "[KTIMER] Error freeing ktimer");
    if (DynListInsertHead(&localTime->freeKTimers, &kTimer->dynListPtrs))
	SystemPanic(0, 0, "[KTIMER] Error releasing ktimer");

}

xm_s32_t ArmKTimer(kTimer_t *kTimer, xmTime_t value, xmTime_t interval) {
    ASSERT(kTimer);
    kTimer->value=value;
    kTimer->interval=interval;
    kTimer->flags|=KTIMER_ARMED;
    SetHwTimer(value);

    return 0;
}

xm_s32_t DisarmKTimer(kTimer_t *kTimer) {
    ASSERT(kTimer);
    if (!(kTimer->flags&KTIMER_ARMED))
	return -1;
    kTimer->flags&=~VTIMER_ARMED;
    return 0;
}

static void VTimerHndl(kTimer_t *kTimer, void *args) {
    kThread_t *k=(kThread_t *)args;
    ASSERT(k->ctrl.g->vClock.flags&VCLOCK_ENABLED);
    ASSERT(k->ctrl.g->vTimer.flags&VTIMER_ARMED);
    CHECK_KTHR_SANITY(k);
    if (k->ctrl.g->vTimer.interval>0)
	k->ctrl.g->vTimer.value+=k->ctrl.g->vTimer.interval;
    else
	k->ctrl.g->vTimer.flags&=~VTIMER_ARMED;

    CLEAR_KTHREAD_FLAG(k, KTHREAD_YIELD_F);
    SetExtIrqPending(k, XM_VT_EXT_EXEC_TIMER);
}

xm_s32_t InitVTimer(vTimer_t *vTimer, void *k) {
    vTimer->kTimer=AllocKTimer(VTimerHndl, k, k);
    return 0;
}

xm_s32_t ArmVTimer(vTimer_t *vTimer, vClock_t *vClock, xmTime_t value, xmTime_t interval) {
    vTimer->value=value;
    
    vTimer->interval=interval;
    vTimer->flags|=VTIMER_ARMED;
    if (vClock->flags&VCLOCK_ENABLED)
	ArmKTimer(vTimer->kTimer, value-vClock->acc+vClock->delta, interval);

    return 0;
}

xm_s32_t DisarmVTimer(vTimer_t *vTimer, vClock_t *vClock) {
    if (!(vTimer->flags&KTIMER_ARMED))
	return -1;
    vTimer->flags&=~KTIMER_ARMED;
    return 0;
}

#define MAX_KTIMERS ((xmcTab.noPartitions*3)+(CONFIG_NO_CPUS*2))

xm_s32_t __VBOOT SetupKTimers(void) {
    localTime_t *localTime=GET_LOCAL_TIME();
    xm_u32_t e;

    DynListInit(&localTime->freeKTimers);
    DynListInit(&localTime->globalActiveKTimers);
    GET_MEMZ(localTime->array, sizeof(kTimer_t)*MAX_KTIMERS);    
    for (e=0; e<MAX_KTIMERS; e++) {
	if (DynListInsertHead(&localTime->freeKTimers, &localTime->array[e].dynListPtrs)) {
	    SystemPanic(0, 0, "[KTIMERS] Error initialising free ktimer list");
	}
    }

    sysHwTimer->SetTimerHandler(TimerHandler);
    return 0;
}

void __VBOOT SetupSysClock(void) {
    if (!sysHwClock||(sysHwClock->InitClock()<0))
	SystemPanic(0, 0, "No system clock available\n");

    kprintf(">> HWClocks [%s (%dKhz)]\n", sysHwClock->name, sysHwClock->freqKhz);
}

void __VBOOT SetupHwTimer(void) {
    if (!sysHwTimer||(sysHwTimer->InitHwTimer()<0))
	SystemPanic(0, 0, "No hwTimer available\n");

    if ((GET_NRCPUS()>1)&&!(sysHwTimer->flags&PER_CPU))
	SystemPanic(0, 0, "No hwTimer available\n");

    kprintf(">> HwTimer [%s (%dKhz)]\n", sysHwTimer->name, sysHwTimer->freqKhz);
}

