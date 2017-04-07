/*
 * $FILE: kthread.h
 *
 * Kernel and Guest kthreads
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_KTHREAD_H_
#define _XM_KTHREAD_H_

#include <assert.h>
#include <guest.h>
#include <ktimer.h>
#include <xmconf.h>
#include <xmef.h>
#include <objdir.h>
#include <arch/kthread.h>
#include <arch/atomic.h>
#include <arch/irqs.h>
#include <arch/xm_def.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

struct guest {
    struct xmcPartition *cfg;
    struct kThreadArch kArch;
    vTimer_t vTimer;
    kTimer_t *kTimer;
    kTimer_t *watchdogTimer;
    vClock_t vClock;
    // this field is accessible by the Guest
    partitionControlTable_t *partitionControlTable;
    xm_u32_t resetCounter;
    xm_u32_t resetStatus;
    struct trapHandler overrideTrapTab[TRAP_NR];
    xm_u32_t opMode;
};

#define CHECK_KTHR_SANITY(k) ASSERT((k->ctrl.magic1==KTHREAD_MAGIC)&&(k->ctrl.magic2==KTHREAD_MAGIC))

#ifdef CONFIG_MMU
#define GET_GUEST_GCTRL_ADDR(k) (k)->ctrl.g->RAM+ROUNDUP2PAGE(sizeof(partitionControlTable_t))
#endif

#ifdef CONFIG_MMULESS
#define GET_GUEST_GCTRL_ADDR(k) ((xmAddress_t)((k)->ctrl.g->partitionControlTable))
#endif

typedef union kThread {
    struct __kThread {
        // Harcoded, don't change it
        xm_u32_t magic1;
        // Harcoded, don't change it
        xmAddress_t *kStack;
        volatile xm_u32_t flags;
        //  [3...0] -> scheduling bits
#define KTHREAD_SV_F (1<<0) // 1:SUPERVISOR
#define KTHREAD_READY_F (1<<1) // 1:READY, 0:SUSPENDED
#define KTHREAD_HALTED_F (1<<2)  // 1:HALTED
#define KTHREAD_YIELD_F (1<<3)
#define   SET_KTHREAD_FLAG(k, f) (k)->ctrl.flags|=(f)
#define   CLEAR_KTHREAD_FLAG(k, f) (k)->ctrl.flags&=~(f)
#define   IS_KTHREAD_FLAG_SET(k, f) ((k)->ctrl.flags&(f))
#define KTHREAD_FP_F (1<<4) // Floating point enabled
        //  [7...5] reserved (should be 0)
        // [15...8] local scheduling bits
#define KTHREAD_SCHED_BIT 8
#define KTHREAD_SCHED_MASK 0xFF00
#define   SET_KTHREAD_SCHED_FLAG(k, f)                                  \
        (k)->ctrl.flags|=(((f)<<KTHREAD_SCHED_BIT)&KTHREAD_SCHED_MASK)
#define   CLEAR_KTHREAD_SCHED_FLAG(k, f)                                \
        (k)->ctrl.flags&=~(((f)<<KTHREAD_SCHED_BIT)&KTHREAD_SCHED_MASK)
#define   IS_KTHREAD_SCHED_FLAG_SET(k, f)                               \
        ((((k)->ctrl.flags&KTHREAD_SCHED_MASK)>>KTHREAD_SCHED_BIT)&(f))
        // [16..15] spare scheduling bits
#define KTHREAD_SPARE_GUEST_F (1<<16)
#define KTHREAD_SPARE_HOST_F (1<<17)
        struct dynList localActiveKTimers;
        struct guest *g;
        void *schedData;
        xm_u32_t magic2;
    } ctrl;
    xm_u8_t kStack[CONFIG_KSTACK_SIZE];
} kThread_t;

extern void InitIdle(kThread_t *idle, xm_s32_t cpu);

extern kThread_t *CreatePartition(struct xmcPartition *conf);
extern xm_s32_t ResetPartition(kThread_t *k, xm_u32_t cold, xm_u32_t status);
extern void ArchCreatePartition(kThread_t *k);
extern void KThreadArchInit(kThread_t *k);
extern void BuildPartitionMemoryImg(kThread_t *k, struct xmPartitionHdr *xmPHdr);
extern partitionControlTable_t *MapPartitionCtrlTab(xmAddress_t partitionCtrlTab);
extern void UnmapPartitionCtrlTab(partitionControlTable_t *partitionCtrlTab);
extern partitionInformationTable_t *MapPartitionInfTab(xmAddress_t partitionInfTab);
extern void UnmapPartitionInfTab(partitionInformationTable_t *partitionInfTab);
extern void FillArchPartitionCtrlTab(kThread_t *k, partitionControlTable_t *partitionCtrlTab);
extern void FillArchPartitionInfTab(kThread_t *k, partitionInformationTable_t *partitionInfTab);
extern void SwitchKThreadArchPre(kThread_t *new, kThread_t *current);
extern void SwitchKThreadArchPost(kThread_t *current);

static inline void SetHwIrqPending(kThread_t *k, xm_s32_t irq) {
    ASSERT(k->ctrl.g);
    ASSERT((irq>=XM_VT_HW_FIRST)&&(irq<=XM_VT_HW_LAST));
    if (IS_KTHREAD_FLAG_SET(k, KTHREAD_HALTED_F))
        return;
    XMAtomicSetMask((1<<irq), &k->ctrl.g->partitionControlTable->hwIrqsPend);
    CLEAR_KTHREAD_FLAG(k, KTHREAD_YIELD_F);
}

static inline void SetExtIrqPending(kThread_t *k, xm_s32_t irq) {
    ASSERT(k->ctrl.g);
    ASSERT((irq>=XM_VT_EXT_FIRST)&&(irq<=XM_VT_EXT_LAST));
    irq-=XM_VT_EXT_FIRST;
#ifdef CONFIG_OBJ_STATUS_ACC
    if (k->ctrl.g)
        partitionStatus[k->ctrl.g->cfg->id].noVIrqs++;
#endif
    if (IS_KTHREAD_FLAG_SET(k, KTHREAD_HALTED_F))
        return;

    XMAtomicSetMask((1<<irq), &k->ctrl.g->partitionControlTable->extIrqsPend);
    CLEAR_KTHREAD_FLAG(k, KTHREAD_YIELD_F);
}

extern void StartUpGuest(void *entry, xmAddress_t stack);

#endif
