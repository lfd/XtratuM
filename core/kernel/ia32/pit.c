/*
 * $FILE: pit.c
 *
 * pit driver
 *
 * $VERSION: 1.0
 *
 * Author: Miguel Masmano <mimastel@doctor.upv.es>
 *
 * $LICENSE:  
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifdef CONFIG_PIT

#include <assert.h>
#include <boot.h>
#include <kdevice.h>
#include <ktimer.h>
#include <stdc.h>
#include <processor.h>
#include <arch/io.h>

// Definitions
#define PIT_IRQ_NR 0
#define PIT_HZ 1193182UL
#define PIT_KHZ 1193UL
#define PIT_ACCURATELY 60

#define PIT_MODE 0x43
#define PIT_CNTR0 0x40
#define PIT_CNTR1 0x41
#define PIT_CNTR2 0x42

RESERVE_HWIRQ(PIT_IRQ_NR);
RESERVE_IOPORTS(0x40, 4);

static hwTimer_t pitTimer;
static timerHandler_t pitHandler;

static inline void PitProgramMode(xm_u8_t m) {
    OutBP(m, PIT_MODE);
}

static inline void PitWriteCounter(xm_u16_t c, xm_u16_t v) {
    OutBP(v&0xff,c);
    OutBP(v>>8, c);
}

static inline void SetPitTimerHwt(xm_u16_t pitCounter) {
    // ONESHOOT_MODE
    PitProgramMode(0x30);
    PitWriteCounter(PIT_CNTR0, pitCounter);
}

static xm_s32_t TimerIrqHandler(irqCtxt_t *ctxt, void *irqData) {
    if (pitHandler)
        (*pitHandler)();
    HwEnableIrq(PIT_IRQ_NR);
    return 0;
}

static xm_s32_t InitPitTimer(void) {
    SetIrqHandler(PIT_IRQ_NR, TimerIrqHandler, 0);
    // setting counter 0 in oneshot mode
    PitProgramMode(0x30);
    pitTimer.flags|=HWTIMER_ENABLED;
    HwEnableIrq(PIT_IRQ_NR);
    
    return 1;
}

static void SetPitTimer(xmTime_t interval) {
    xm_u16_t pitCounter=(interval*PIT_HZ)/USECS_PER_SEC;
    SetPitTimerHwt(pitCounter);
}

static xmTime_t GetPitTimerMaxInterval(void) {
    return 54924; //(0xF0*USECS_PER_SEC)/PIT_HZ;
}

static xmTime_t GetPitTimerMinInterval(void) {
    return PIT_ACCURATELY;
}

static timerHandler_t SetPitTimerHandler(timerHandler_t TimerHandler) {
    timerHandler_t OldPitUserHandler=pitHandler;
    pitHandler=TimerHandler;
    return OldPitUserHandler;
}

static void PitTimerShutdown(void) {
    pitTimer.flags&=~HWTIMER_ENABLED;
    HwDisableIrq(PIT_IRQ_NR);
    SetIrqHandler(PIT_IRQ_NR, TimerIrqHandler, 0);
}

static hwTimer_t pitTimer={
    .name="i8253 timer",
    .flags=0,
    .freqKhz=PIT_KHZ,
    .InitHwTimer=InitPitTimer,
    .SetHwTimer=SetPitTimer,
    .GetMaxInterval=GetPitTimerMaxInterval,
    .GetMinInterval=GetPitTimerMinInterval,
    .SetTimerHandler=SetPitTimerHandler,
    .ShutdownHwTimer=PitTimerShutdown,
};

hwTimer_t *sysHwTimer=&pitTimer;

#endif //CONFIG_PIT
