/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel
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
#include <kdevice.h>
#include <ktimer.h>
#include <stdc.h>
#include <irqs.h>
#include <objdir.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <kthread.h>
#include <vmmap.h>
#include <virtmm.h>
#include <xmconf.h>

#include <objects/console.h>
#include <objects/status.h>
#include <arch/paging.h>
#include <arch/xm_def.h>

// CPU's frequency
xm_u32_t cpuKhz;

struct xmcPartition *xmcPartitionTab;
struct xmcMemoryRegion *xmcMemRegTab;
struct xmcMemoryArea *xmcPhysMemAreaTab;
struct xmcCommChannel *xmcCommChannelTab;
struct xmcCommPort *xmcCommPorts;
struct xmcIoPort *xmcIoPortTab;
struct xmcSchedCyclicSlot *xmcSchedCyclicSlotTab;
struct xmcSchedCyclicPlan *xmcSchedCyclicPlanTab;
char *xmcStringTab;

#ifdef CONFIG_DEV_MEMBLOCK
struct xmcMemBlock *xmcMemBlockTab;
#endif

// Local info
localCpu_t localCpuInfo[CONFIG_NO_CPUS];
localTime_t localTimeInfo[CONFIG_NO_CPUS];
localSched_t localSchedInfo[CONFIG_NO_CPUS];

static volatile xm_s32_t procWaiting __VBOOTDATA=0;

extern __NOINLINE__ void FreeBootMem(void);

void IdleTask(void) {
    extern void (*Idle)(void);
    localCpu_t *cpu = GET_LOCAL_CPU();

    while(1) {
        HwCli();
        if(cpu->irqNestingCounter==SCHED_PENDING)
            Scheduling();
        HwSti();
        Idle(); // In the IA32 Arch, force STI to take effect
        }
}

void HaltSystem(void) {
    kprintf("System halted.\n");
    __HaltSystem();
}

static void __VBOOT CreateLocalInfo(void) {
    if (!GET_NRCPUS())
        SystemPanic(0, 0, "No cpu found in the system\n");
    memset(localCpuInfo, 0, sizeof(localCpu_t)*CONFIG_NO_CPUS);
    memset(localTimeInfo, 0, sizeof(localTime_t)*CONFIG_NO_CPUS);
    memset(localSchedInfo, 0, sizeof(localSched_t)*CONFIG_NO_CPUS);
}

static void __VBOOT LocalSetup(xm_s32_t cpuId, kThread_t *idle) {
    ASSERT(!HwIsSti());
    ASSERT(xmcTab.hpv.noCpus>cpuId);
    SetupCpu();
    SetupArchLocal(cpuId);
    SetupHwTimer();
    SetupKTimers();
    InitSchedLocal(idle, xmcTab.hpv.cpuTab[cpuId].schedPolicy, &xmcTab.hpv.cpuTab[cpuId].schedParams);
}

static void __VBOOT SetupPartitions(void) {
    kThread_t *k;
    xm_s32_t e, a;
    kprintf("%d Partition(s) created\n", xmcTab.noPartitions);

    // Creating the partitions
    for (e = 0; e < xmcTab.noPartitions; e++) {
        if ((k = CreatePartition(&xmcPartitionTab[e]))) {
            kprintf("P%d (\"%s\":%d) cpu: %d flags: [", e, &xmcStringTab[xmcPartitionTab[e].nameOffset], xmcPartitionTab[e].id, xmcPartitionTab[e].flags & XM_PART_CPU_MASK);
            if (xmcPartitionTab[e].flags & XM_PART_SUPERVISOR)
                kprintf(" SV");
#ifdef CONFIG_SPARE_SCHEDULING
            if (xmcPartitionTab[e].flags&XM_PART_SPARE_G)
            kprintf(" SPGUEST");
            if (xmcPartitionTab[e].flags&XM_PART_SPARE_H)
            kprintf(" SPHOST");
#endif
            if (xmcPartitionTab[e].flags & XM_PART_BOOT)
                kprintf(" BOOT (0x%x)", xmcPartitionTab[e].loadPhysAddr + xmcPartitionTab[e].headerOffset);
            kprintf(" ]:\n");
            for (a = 0; a < xmcPartitionTab[e].noPhysicalMemoryAreas; a++) {
                kprintf("    [0x%lx", xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].startAddr);
                kprintf(" - 0x%lx", xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].startAddr+xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].size);

                kprintf("]\n");
            }
        } else {
            PartitionPanic(0, 0, "[LoadGuests] Error creating partition");
        }
        AddKThread(k, k->ctrl.g->cfg->flags & XM_PART_CPU_MASK);
    }
}

static void __VBOOT LoadCfgTab(void) {
    // Check configuration file
    if (xmcTab.signature!=XMC_SIGNATURE)
        HaltSystem();
    
    xmcPartitionTab=(struct xmcPartition *)(xmcTab.partitionTabOffset+(xmAddress_t)&xmcTab);
    xmcMemRegTab=(struct xmcMemoryRegion *)(xmcTab.memoryRegionsOffset+(xmAddress_t)&xmcTab);
    xmcPhysMemAreaTab=(struct xmcMemoryArea *)(xmcTab.physicalMemoryAreasOffset+(xmAddress_t)&xmcTab);
    xmcCommChannelTab=(struct xmcCommChannel *)(xmcTab.commChannelTabOffset+(xmAddress_t)&xmcTab);
    xmcCommPorts=(struct xmcCommPort *)(xmcTab.commPortsOffset+(xmAddress_t)&xmcTab);
    xmcIoPortTab=(struct xmcIoPort *)(xmcTab.ioPortsOffset+(xmAddress_t)&xmcTab);
    xmcSchedCyclicSlotTab=(struct xmcSchedCyclicSlot *)(xmcTab.schedCyclicSlotsOffset+(xmAddress_t)&xmcTab);
    xmcSchedCyclicPlanTab=(struct xmcSchedCyclicPlan *)(xmcTab.schedCyclicPlansOffset+(xmAddress_t)&xmcTab);
    xmcStringTab=(char *)(xmcTab.stringsOffset+(xmAddress_t)&xmcTab);

#ifdef CONFIG_DEV_MEMBLOCK
    xmcMemBlockTab=(struct xmcMemBlock *)(xmcTab.deviceTab.memBlocksOffset+(xmAddress_t)&xmcTab);
#endif

    // MISS:
    // Check revision and checksum
}

static void __VBOOT BootPartitions(void) {
    xm_s32_t e;
    kThread_t *k;

    for (e=0; e<xmcTab.noPartitions; e++) {
        k=partitionTab[e];
        // Booting the partitions with the flag boot enabled
        if (k&&(k->ctrl.g->cfg->flags&XM_PART_BOOT))
            if (ResetPartition(k, 1, 0)<0)
                kprintf("Unable to reset this partition\n");
        k->ctrl.g->opMode=XM_OPMODE_IDLE;
    }
}

/* 
   XM's first C function
   XXX: XM is halted when this function exits
*/
void __VBOOT Setup(xm_s32_t cpuId, kThread_t *idle) {
    ConsoleInit(GetEarlyOutput());
    // irqs _must_ be disabled
    ASSERT(!HwIsSti());
    kprintf("XM Hypervisor (%x.%x r%x)\n", (XM_VERSION>>16)&0xFF, (XM_VERSION>>8)&0xFF, XM_VERSION&0xFF);
    LoadCfgTab();
    InitBrk((xmAddress_t)&xmcTab+xmcTab.size);
    EarlySetupCpu();
    SetupIrqs();
    cpuKhz=GetCpuKhz();
    kprintf("Detected %lu.%luMHz processor.\n", (xm_u32_t)(cpuKhz/1000), (xm_u32_t)(cpuKhz%1000));
    // Configuring the physical and the virtual memory managers
    SetupPhysMM();
    SetupVirtMM(XM_START_VMMAP, XM_END_VMMAP);
    ZeroPhysMM();   /* Patch for XtratuM size > 4MB */
    SetupKDev();
    SetupObjDir();
    InitSched();
    EarlySetupArchCommon();
    CreateLocalInfo();
    SetupArchCommon();
    SetupSysClock();
    LocalSetup(cpuId, idle);
    SetupPartitions();
    /*
      proc_waiting++;
      while(proc_waiting<GET_NRCPU());*/
    BootPartitions();
    FreeBootMem();
}
