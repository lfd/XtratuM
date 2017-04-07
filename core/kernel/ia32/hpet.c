/*
 * $FILE: hpet.c
 *
 * High Precision Event Timer
 *
 * $VERSION: 1.0
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifdef CONFIG_HPET

#include <assert.h>
#include <boot.h>
#include <kdevice.h>
#include <ktimer.h>
#include <stdc.h>
#include <processor.h>
#include <virtmm.h>
#include <vmmap.h>
#include <arch/io.h>

#define HPET_IRQ_NR             0
#define HPET_HZ                 14318180UL
#define HPET_KHZ                14318UL

#define HPET_PHYS_ADDR          0xfed00000

#define HPET_GCID               (0x000)
#define     HPET_GCID_VID       0xffff0000
#define     HPET_VENDOR_ID(ID)  ((ID)<<16)
#define     HPET_GCID_LRC       0x00008000
#define     HPET_GCID_CS        0x00002000
#define     HPET_GCID_NT        0x00001f00
#define     HPET_GCID_RID       0x000000ff

#define HPET_CTP                (0x004)

#define HPET_GCFG               (0x010)
#define     HPET_GCFG_LRE       0x00000002
#define     HPET_GCFG_EN        0x00000001

#define HPET_GIS                (0x020)
#define     HPET_GIS_T2         0x00000004
#define     HPET_GIS_T1         0x00000002
#define     HPET_GIS_T0         0x00000001

#define HPET_MCV                (0x0f0)
#define     HPET_MCV_L          (0x0f0)
#define     HPET_MCV_H          (0x0f4)

#define HPET_T0CC               (0x100)
#define HPET_T1CC               (0x120)
#define HPET_T2CC               (0x140)
#define     HPET_TnCC_IT        0x00000002
#define     HPET_TnCC_IE        0x00000004
#define     HPET_TnCC_TYP       0x00000008
#define     HPET_TnCC_PIC       0x00000010
#define     HPET_TnCC_TS        0x00000020
#define     HPET_TnCC_TVS       0x00000040
#define     HPET_TnCC_T32M      0x00000100
#define     HPET_TnCC_IR(N)     (((N)&0x1f)<<9)
#define     HPET_TnCC_IMASK     (0x1f<<9)

#define HPET_T0ROUTE            (0x104)
#define HPET_T1ROUTE            (0x124)
#define HPET_T2ROUTE            (0x144)

#define HPET_T0CV               (0x108)
#define     HPET_T0CV_L         (0x108)
#define     HPET_T0CV_H         (0x10C)

#define HPET_T1CV               (0x128)
#define     HPET_T1CV_L         (0x128)
#define     HPET_T1CV_H         (0x12C)

#define HPET_T2CV               (0x148)
#define     HPET_T2CV_L         (0x148)
#define     HPET_T2CV_H         (0x14C)

static hwTimer_t hpetTimer;
static timerHandler_t hpetHandler;
static xmAddress_t hpetVirtAddr;

RESERVE_HWIRQ(HPET_IRQ_NR);
RESERVE_PHYSPAGES(HPET_PHYS_ADDR, 1);

static inline xm_u32_t hpetReadReg(xm_u32_t reg) {
    xm_u32_t ret;
    __asm__ __volatile__("movl %1, %0" : "=r" (ret) : "m" (*(volatile xm_u32_t *)(hpetVirtAddr+reg)) :"memory");
    return ret;
}

static inline void hpetWriteReg(xm_u32_t reg, xm_u32_t val) {
    __asm__ __volatile__("movl %0, %1": :"r" (val), "m" (*(volatile xm_u32_t *)(hpetVirtAddr+reg)) :"memory");
}

static xm_s32_t TimerIrqHandler(irqCtxt_t *ctxt, void *irqData) {
    if (hpetHandler)
        (*hpetHandler)();
    HwEnableIrq(HPET_IRQ_NR);
    return 0;
}

static xm_s32_t InitHpetTimer(void) {
    xm_u32_t cfg;

    // Map the HPET base address to a virtual address accessible by XtratuM
    hpetVirtAddr = VmmAlloc(1);
    VmMapPage(HPET_PHYS_ADDR, hpetVirtAddr, _PG_PRESENT|_PG_RW);

    // Check if we have mapped correctly the timer page
    if ((HPET_GCID_VID & hpetReadReg(HPET_GCID)) != HPET_VENDOR_ID(CONFIG_HPET_VENDOR_ID)) {
        kprintf("Bad HPET Vendor ID: %x\n", (HPET_GCID_VID & hpetReadReg(HPET_GCID)));
        return -1;
    }
    kprintf("HPET mapped to address %x\n", hpetVirtAddr);

    // Setup IRQ (Reserved IRQ0)
    SetIrqHandler(HPET_IRQ_NR, TimerIrqHandler, 0);
    cfg = HPET_TnCC_IR(HPET_IRQ_NR) | HPET_TnCC_IE | HPET_TnCC_T32M;
    hpetWriteReg(HPET_T0CC, cfg);
    if ((hpetReadReg(HPET_T0CC) & HPET_TnCC_IMASK) != HPET_TnCC_IR(HPET_IRQ_NR)) {
        kprintf("HPET interrupt not routed\n");
        return -1;
    }

    hpetTimer.flags|=HWTIMER_ENABLED;
    HwEnableIrq(HPET_IRQ_NR);

    // Enable timer
    cfg = hpetReadReg(HPET_GCFG);
    cfg |= HPET_GCFG_EN | HPET_GCFG_LRE;
    hpetWriteReg(HPET_GCFG, cfg);

    return 1;
}

static void SetHpetTimer(xmTime_t interval) {
    xm_u32_t hpetCounter=(interval*HPET_HZ)/USECS_PER_SEC;
    xm_u32_t cnt;

    // Overflow is not a problem. The counter will wrap and generate an interrupt
    // in the same interval
    cnt = hpetReadReg(HPET_MCV);
    cnt += hpetCounter;
    hpetWriteReg(HPET_T0CV, cnt);
}

static xmTime_t GetHpetTimerMaxInterval(void) {
    return (0xFFFFFFFFULL*USECS_PER_SEC)/HPET_HZ;
}

static xmTime_t GetHpetTimerMinInterval(void) {
    return 2;   // 2 us shot
}

static timerHandler_t SetHpetTimerHandler(timerHandler_t TimerHandler) {
    timerHandler_t OldHpetUserHandler=hpetHandler;
    hpetHandler=TimerHandler;
    return OldHpetUserHandler;
}

static void HpetTimerShutdown(void) {
    xm_u32_t cfg;
    hpetTimer.flags&=~HWTIMER_ENABLED;
    HwDisableIrq(HPET_IRQ_NR);
    SetIrqHandler(HPET_IRQ_NR, TimerIrqHandler, 0);

    cfg = hpetReadReg(HPET_GCFG);
    cfg &= ~HPET_GCFG_EN;
    hpetWriteReg(HPET_GCFG, cfg);
}

static hwTimer_t hpetTimer={
    .name="HPET timer",
    .flags=0,
    .freqKhz=HPET_KHZ,
    .InitHwTimer=InitHpetTimer,
    .SetHwTimer=SetHpetTimer,
    .GetMaxInterval=GetHpetTimerMaxInterval,
    .GetMinInterval=GetHpetTimerMinInterval,
    .SetTimerHandler=SetHpetTimerHandler,
    .ShutdownHwTimer=HpetTimerShutdown,
};

hwTimer_t *sysHwTimer=&hpetTimer;

#endif //CONFIG_HPET
