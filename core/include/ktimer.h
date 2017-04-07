/*
 * $FILE: ktimer.h
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

#ifndef _XM_KTIMERS_H_
#define _XM_KTIMERS_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <list.h>

#define NSECS_PER_SEC 1000000000ULL
#define USECS_PER_SEC 1000000UL

typedef xm_s32_t (*timerHandler_t)(void);

typedef struct hwClock {
    char *name;
    xm_u32_t flags;
#define HWCLOCK_ENABLED (1<<0)
#define PER_CPU (1<<1)
    xm_u32_t freqKhz;
    xm_s32_t (*InitClock)(void);
    xmTime_t (*GetTimeUsec)(void);
    hwTime_t (*GetTimeHwt)(void);
    void (*ShutdownClock)(void);
} hwClock_t;

extern hwClock_t *sysHwClock;

// The following structure defines each hwtimer existing in the system
typedef struct hwTimer {
    char *name;
    xm_u32_t flags;
#define HWTIMER_ENABLED (1<<0)
    xm_u32_t freqKhz;    
    xm_s32_t irq;
    xm_s32_t (*InitHwTimer)(void);
    void (*SetHwTimer)(xmTime_t);
    // This is the maximum value to be programmed
    xmTime_t (*GetMaxInterval)(void);
    xmTime_t (*GetMinInterval)(void);
    timerHandler_t (*SetTimerHandler)(timerHandler_t);
    void (*ShutdownHwTimer)(void);
} hwTimer_t;

extern hwTimer_t *sysHwTimer;

typedef struct kTimer {
    struct dynListNode dynListPtrs; // hard-coded, don't touch
    hwTime_t value;
    hwTime_t interval;
    xm_u32_t flags;
#define KTIMER_ARMED (1<<0)
    void *actionArgs;
    void (*Action)(struct kTimer *, void *);
} kTimer_t;

typedef struct {
    xmTime_t value;
    xmTime_t interval;
#define VTIMER_ARMED (1<<0)
    xm_u32_t flags;
    kTimer_t *kTimer;
} vTimer_t;

typedef struct {
    xmTime_t acc;
    xmTime_t delta;
    xm_u32_t flags;
#define VCLOCK_ENABLED (1<<0)
} vClock_t;

typedef struct {
    xm_u32_t flags;
#define KTIMER_HANDLER_PENDING 0x1
#define NEXT_ACTIVATION_VALID 0x2
    xmTime_t nextAct;
    kTimer_t *array;
    struct dynList freeKTimers, globalActiveKTimers;
} localTime_t;

extern localTime_t localTimeInfo[];

#define GET_LOCAL_TIME() (&localTimeInfo[GET_CPU_ID()])

extern xm_s32_t SetupKTimers(void);
extern void SetupSysClock(void);
extern void SetupHwTimer(void);

static inline xmTime_t GetSysClockUsec(void) {
    return sysHwClock->GetTimeUsec();
}

static inline xmTime_t GetSysClock(void) {
    return sysHwClock->GetTimeHwt();
}

static inline xmTime_t GetTimeUsecVClock(vClock_t *vClock) {
    xmTime_t t=vClock->acc;
    if (vClock->flags&VCLOCK_ENABLED)
	t+=(GetSysClockUsec()-vClock->delta);
	    
    return t;
}

extern kTimer_t *AllocKTimer(void (*act)(kTimer_t *, void *), void *args, void *kThread);
extern void FreeKTimer(kTimer_t *kTimer, void *kThread);
extern xm_s32_t ArmKTimer(kTimer_t *kTimer, xmTime_t value, xmTime_t interval);
extern xm_s32_t DisarmKTimer(kTimer_t *kTimer);
extern xm_s32_t InitVTimer(vTimer_t *vTimer, void *k);
extern xm_s32_t ArmVTimer(vTimer_t *vTimer, vClock_t *vClock, xmTime_t value, xmTime_t interval);
extern xm_s32_t DisarmVTimer(vTimer_t *vTimer, vClock_t *vClock);
extern xm_s32_t Udelay(xm_u32_t usec);
extern void TimerHandlerBottom(void);
extern void SetNextTimerActivation(void);

static inline void InitVClock(vClock_t *vClock) {
    vClock->acc=0;
    vClock->delta=0;
    vClock->flags=0;
}

static inline void StopVClock(vClock_t *vClock, vTimer_t *vTimer) {
    if (vTimer->flags&VTIMER_ARMED)
	DisarmKTimer(vTimer->kTimer);

    vClock->flags&=(~VCLOCK_ENABLED);
    vClock->acc+=GetSysClockUsec()-vClock->delta;
}

static inline void ResumeVClock(vClock_t *vClock, vTimer_t *vTimer) {
    vClock->delta=GetSysClockUsec();
    vClock->flags|=VCLOCK_ENABLED;
    
    if (vTimer->flags&VTIMER_ARMED)
	ArmKTimer(vTimer->kTimer, vTimer->value-vClock->acc+vClock->delta, vTimer->interval);
}

static inline xmTime_t HwTime2Duration(hwTime_t t, hwTime_t hz) {
    hwTime_t s;
    s=t/hz;
    return (s*USECS_PER_SEC+((t-s*hz)*USECS_PER_SEC)/hz);
}

static inline hwTime_t Duration2HwTime(xmTime_t d, hwTime_t hz) {
    hwTime_t s;
    s=d/USECS_PER_SEC;
    return s*hz+((d-s*USECS_PER_SEC)*hz)/USECS_PER_SEC;
}

#endif
