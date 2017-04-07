/*
 * $FILE: kthread.c
 *
 * Kernel and Guest context
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
#include <ktimer.h>
#include <kthread.h>
#include <physmm.h>
#include <sched.h>
#include <stdc.h>
#include <virtmm.h>
#include <vmmap.h>
#include <xmef.h>
#include <arch/cswitch.h>
#include <arch/xm_def.h>

static kThread_t *kThreadPool;
static struct guest *guestPool;

static void KThrTimerHndl(kTimer_t *kTimer, void *args) {
    kThread_t *k=(kThread_t *)args;

    CHECK_KTHR_SANITY(k);
    CLEAR_KTHREAD_FLAG(k, KTHREAD_YIELD_F);
    SetExtIrqPending(k, XM_VT_EXT_HW_TIMER);
}

static void KThrWatchdogTimerHndl(kTimer_t *kTimer, void *args) {
    kThread_t *k=(kThread_t *)args;
    xmHmLog_t hmLog;

    CHECK_KTHR_SANITY(k);
    CLEAR_KTHREAD_FLAG(k, KTHREAD_YIELD_F);
    SetExtIrqPending(k, XM_VT_EXT_WATCHDOG_TIMER);
    hmLog.partitionId=k->ctrl.g->cfg->id;
    hmLog.system=0;
    hmLog.eventId=XM_HM_EV_WATCHDOG_TIMER;
    HmRaiseEvent(&hmLog);    
}

void InitIdle(kThread_t *idle, xm_s32_t cpu) {
    GET_MEMZ(kThreadPool, sizeof(kThread_t)*xmcTab.noPartitions);
    GET_MEMZ(guestPool, sizeof(struct guest)*xmcTab.noPartitions);
    idle->ctrl.magic1=idle->ctrl.magic2=KTHREAD_MAGIC;
    DynListInit(&idle->ctrl.localActiveKTimers);
    idle->ctrl.kStack=0;
    idle->ctrl.g=0;
    SET_KTHREAD_FLAG(idle, KTHREAD_READY_F);
    SET_KTHREAD_FLAG(idle, KTHREAD_SV_F);
}

void StartUpGuest(void *entry, xmAddress_t stack) {
    localSched_t *sched=GET_LOCAL_SCHED();

    //KTHREAD_ARCH_INIT(sched->cKThread, stack);
    KThreadArchInit(sched->cKThread);
    XMAtomicSet(&sched->cKThread->ctrl.g->partitionControlTable->iFlags, 0);
    ResumeVClock(&sched->cKThread->ctrl.g->vClock, &sched->cKThread->ctrl.g->vTimer);

    // JMP_PARTITION must enable interrupts
    JMP_PARTITION(entry);

    PartitionPanic(0, 0, __FILE__":%u:0x%x: executing unreachable code!", __LINE__, sched->cKThread);
}

static inline kThread_t *AllocPartition(struct xmcPartition *conf) {
    kThread_t *k;
    ASSERT((conf->id>=0)&&(conf->id<xmcTab.noPartitions));
    k=&kThreadPool[conf->id];
    k->ctrl.g=&guestPool[conf->id];
    k->ctrl.magic1=k->ctrl.magic2=KTHREAD_MAGIC;
    DynListInit(&k->ctrl.localActiveKTimers);
    return k;
}

kThread_t *CreatePartition(struct xmcPartition *conf) {
    kThread_t *k=AllocPartition(conf);

    ASSERT(conf);
    if (conf->flags&XM_PART_SUPERVISOR)    
        SET_KTHREAD_FLAG(k, KTHREAD_SV_F);

    if (conf->flags&XM_PART_FP)
        SET_KTHREAD_FLAG(k, KTHREAD_FP_F);

#ifdef CONFIG_SPARE_SCHEDULING
    if (conf->flags&XM_PART_SPARE_G)
        SET_KTHREAD_FLAG(k, KTHREAD_SPARE_GUEST_F);
    if (conf->flags&XM_PART_SPARE_H)
        SET_KTHREAD_FLAG(k, KTHREAD_SPARE_HOST_F);
#endif

    k->ctrl.g->kTimer=AllocKTimer(KThrTimerHndl, k, k);
    k->ctrl.g->watchdogTimer=AllocKTimer(KThrWatchdogTimerHndl, k, k);
    InitVTimer(&k->ctrl.g->vTimer, k);
    k->ctrl.g->cfg=conf;
    ArchCreatePartition(k);
    partitionTab[k->ctrl.g->cfg->id]=k;
    HALT_PARTITION(k->ctrl.g->cfg->id);

    return k;
}

static inline void FillPartInfTab(partitionInformationTable_t *partInfTab, struct xmcPartition *cfg) {
    xm_s32_t e;
    partInfTab->signature=PARTITION_INFORMATION_TABLE_SIGNATURE;
    partInfTab->xmVersion=XM_VERSION;
    partInfTab->xmAbiVersion=XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION);
    partInfTab->xmApiVersion=XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION);
    partInfTab->id=cfg->id;
    partInfTab->cpuKhz=cpuKhz;
    partInfTab->flags=cfg->flags;
    for (e=0; e<CONFIG_NO_HWIRQS; e++)
        if (xmcTab.hpv.hwIrqTab[e].owner==cfg->id)
            partInfTab->hwIrqs|=(1<<e);
    strcpy(partInfTab->name, &xmcStringTab[cfg->nameOffset]);
    partInfTab->noPartitions=xmcTab.noPartitions;
    partInfTab->noPhysicalMemoryAreas=cfg->noPhysicalMemoryAreas;
    //memAreaTab=(struct xmcMemoryArea *)((xmAddress_t)partInfTab+sizeof(partitionInformationTable_t));
    //partInfTab->physicalMemoryAreasOffset=sizeof(partitionInformationTable_t);
 
    //memcpy(memAreaTab, &xmcPhysMemAreaTab[cfg->physicalMemoryAreasOffset], sizeof(struct xmcMemoryArea)*cfg->noPhysicalMemoryAreas);
}
#define CHECKPOINT()    kprintf("[%s:%d]\n", __FUNCTION__, __LINE__)
xm_s32_t ResetPartition(kThread_t *k, xm_u32_t cold, xm_u32_t status) {
    partitionInformationTable_t *partInfTab;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmPartitionHdr *xmPHdr=0;
    struct xmImageHdr *xmIHdr;
    struct physPage *partHdr=0, *imgHdr;
    xmAddress_t ePoint;
    xmHmLog_t hmLog;

    ASSERT(k->ctrl.g);
    if (!(imgHdr=PmmFindPage(k->ctrl.g->cfg->loadPhysAddr+k->ctrl.g->cfg->headerOffset, k, 0)))
        return -1;
    xmIHdr=VCacheMapPage(k->ctrl.g->cfg->loadPhysAddr+k->ctrl.g->cfg->headerOffset, imgHdr);
    if (xmIHdr->signature==XMEF_PARTITION_MAGIC) {
        if (!(partHdr=PmmFindPage((xmAddress_t)xmIHdr->entry.defaultPartitionHdr, k, 0)))
            return -1;
        xmPHdr=VCacheMapPage((xmAddress_t)xmIHdr->entry.defaultPartitionHdr, partHdr);
        if (xmPHdr->signature!=XMEF_PARTITION_HDR_MAGIC) {
            goto _exitOnError;
        }
    } else if (xmIHdr->signature==XMEF_PARTITION_HDR_MAGIC) {
        partHdr=imgHdr;
        xmPHdr=(struct xmPartitionHdr *)xmIHdr;
        if (!(imgHdr=PmmFindPage((xmAddress_t)xmPHdr->imageHdr, k, 0)))
            return -1;
        xmIHdr=VCacheMapPage((xmAddress_t)xmPHdr->imageHdr, imgHdr);
        if (xmIHdr->signature!=XMEF_PARTITION_MAGIC) {
            goto _exitOnError;
        }
    } else 
        goto _exitOnError;

    if (xmIHdr->xmAbiVersion!=XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION)) goto _exitOnError;
    if(xmIHdr->xmApiVersion!=XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION)) goto _exitOnError;

    BuildPartitionMemoryImg(k, xmPHdr);
    UnmapPartitionCtrlTab(k->ctrl.g->partitionControlTable);
    k->ctrl.g->partitionControlTable=MapPartitionCtrlTab((xmAddress_t)xmPHdr->partitionControlTable);

    if (cold) {
        memset(k->ctrl.g->partitionControlTable, 0, sizeof(partitionControlTable_t));
        k->ctrl.g->partitionControlTable->magic=KTHREAD_MAGIC;
        FillArchPartitionCtrlTab(k, k->ctrl.g->partitionControlTable);
    }
    partInfTab=MapPartitionInfTab((xmAddress_t)xmPHdr->partitionInformationTable);
    if (cold) {
        memset(partInfTab, 0, sizeof(partitionInformationTable_t));
        k->ctrl.g->resetCounter=0;
        FillPartInfTab(partInfTab, k->ctrl.g->cfg);
        FillArchPartitionInfTab(k, partInfTab);
    } else {
        k->ctrl.g->resetCounter++;
        partInfTab->resetCounter=k->ctrl.g->resetCounter;
    }
    k->ctrl.g->resetStatus=partInfTab->resetStatus=status;
    partInfTab->checksum=xmCalcChecksum(partInfTab, sizeof(partitionInformationTable_t));
    UnmapPartitionInfTab(partInfTab);
    XMAtomicSet(&k->ctrl.g->partitionControlTable->extIrqsMask, ~0);

    SET_KTHREAD_FLAG(k, KTHREAD_READY_F);
    CLEAR_KTHREAD_FLAG(k, KTHREAD_HALTED_F);
    ePoint=xmPHdr->ePoint;
    VCacheUnlockPage(imgHdr);
    VCacheUnlockPage(partHdr);
    
    if (sched->cKThread!=k)
        SETUP_KSTACK(k, StartUpGuest, ePoint);
    else
        StartUpGuest((void *)ePoint, (xmAddress_t)k->ctrl.kStack);

    return 0;
_exitOnError:
    memset(&hmLog, 0, sizeof(xmHmLog_t));
    hmLog.eventId=XM_HM_EV_INCOMPATIBLE_INTERFACE;
    hmLog.partitionId=k->ctrl.g->cfg->id;
    hmLog.system=1;
    if (xmPHdr) {
        hmLog.word[1]=xmPHdr->signature;
        VCacheUnlockPage(partHdr);
    }
    if (xmIHdr) {
        hmLog.word[0]=xmIHdr->signature;
        hmLog.word[2]=xmIHdr->xmAbiVersion;
        hmLog.word[3]=xmIHdr->xmApiVersion;
        VCacheUnlockPage(imgHdr);
    }
    HmRaiseEvent(&hmLog);
    return -1;
}

