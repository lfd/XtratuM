/*
 * $FILE: sched.h
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

#ifndef _XM_SCHED_H_
#define _XM_SCHED_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <kthread.h>

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
#ifdef CONFIG_SPARE_SCHEDULING
#include <objects/spare.h>
extern xm_s32_t SetSparePlan(struct xmcSchedSparePlan *newSparePlan);
extern struct xmcSchedSparePlan *GetCurrentSparePlan(void);
extern xm_s32_t FillSparePartitionTab(struct xmcSchedSparePlan *spareData);
extern xm_s32_t LaunchSparePlan(void);
#endif

#endif
