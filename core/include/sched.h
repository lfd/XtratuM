/*
 * $FILE: sched.h
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

#ifndef _XM_SCHED_H_
#define _XM_SCHED_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <kthread.h>
#include <objects/status.h>

struct cyclicData {
    kTimer_t *kTimer;
    struct {
        const struct xmcSchedCyclicPlan *current;
        const struct xmcSchedCyclicPlan *new;
        const struct xmcSchedCyclicPlan *prev;
    } plan;
    const struct xmcSchedCyclic *cfgTab;
    xm_s32_t slot; // next slot to be processed
    xm_u32_t newSlot;
    xmTime_t mjf;
    xmTime_t sExec;
    xmTime_t planSwitchTime;
    xmTime_t csStart, csEnd; // Current slot stats
};

typedef struct {
    kThread_t *idleKThread;
    kThread_t *cKThread;
    kThread_t *fpuOwner;
    struct scheduler *scheduler;
    struct cyclicData *schedulerData;
    xm_u32_t flags;
#define LOCAL_SCHED_ENABLED 0x1
} localSched_t;

extern localSched_t localSchedInfo[];

#define GET_LOCAL_SCHED() \
    (&localSchedInfo[GET_CPU_ID()])

struct scheduler {
    char *name;
    xm_s32_t (*Init)(kThread_t *idle, const union xmcSchedParams *schedParams);
    kThread_t *(*GetReadyKThread)(kThread_t *ckThread, kThread_t *idle, void *data);
    void (*AddKThread)(kThread_t *k);
    void (*Shutdown)(void);
};

#define SCHED_NR 1
extern struct scheduler *schedList[SCHED_NR];

extern kThread_t **partitionTab;
extern xm_s32_t nrPartitions;

extern void InitSched(void);
extern void InitSchedLocal(kThread_t *idle, xm_s32_t sched, const union xmcSchedParams *schedParams);
extern void Scheduling(void);
extern void SetSchedPending(void);
extern xm_s32_t SwitchSchedPlan(xm_s32_t newPlanId, xm_s32_t *oldPlanId);
extern xm_s32_t AddKThread(kThread_t *k, xm_s32_t cpu);

static inline void SUSPEND_PARTITION(xmId_t id) {
    CLEAR_KTHREAD_FLAG(partitionTab[id], KTHREAD_READY_F); // KTHREAD_SUSPENDED_F
}

static inline void RESUME_PARTITION(xmId_t id) {
    SET_KTHREAD_FLAG(partitionTab[id], KTHREAD_READY_F); // KTHREAD_SUSPENDED_F
}

static inline void IDLE_PARTITION(xmId_t id) {
    CLEAR_KTHREAD_FLAG(partitionTab[id], KTHREAD_READY_F);
}

static inline void SHUTDOWN_PARTITION(xmId_t id) {
    SetExtIrqPending(partitionTab[id], XM_VT_EXT_SHUTDOWN);
}

static inline void HALT_PARTITION(xmId_t id) {
    partitionTab[id]->ctrl.g->opMode=XM_OPMODE_IDLE;
    SET_KTHREAD_FLAG(partitionTab[id], KTHREAD_HALTED_F);
}

#ifdef CONFIG_SPARE_SCHEDULING
struct xmcSpareHeader {
    xmId_t partitionId;
#define XM_SPARE_STATUS_BUSY (1<<0)
    xm_u32_t status;
};
extern void SetSpareGuest(kThread_t *guest);
#endif

#endif
