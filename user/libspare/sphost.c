/*
 * $FILE: sphost.c
 *
 * Spare host partition
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */
#include <irqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spare.h>
#include <xm.h>

#define PARTITION_ACTIVE    (1<<0)
#define ROUNDUP(x, a)       (((x) + ((a)-1)) & (-a))

struct Scheduler {
    char *name;
    void (*init)(void);
    xm_s32_t (*next)(void);
};

struct Partition {
    xm_u32_t status;
    struct SpareHeader *header;
    struct SchedData *schedData;
};

__attribute__((section(".inputSchedData"))) struct SchedData inputSchedData[128];

extern xm_u32_t _eguest[];
static xmAddress_t poolPtr, poolEnd;
static xm_s32_t noPartitions, cPart, cSched;
static struct Partition *partitionTab;

static xm_u32_t nextIdle, newSlot;
xmTime_t slotStart, slotEnd, pSlotEnd;

xm_s32_t SchedPriorityNext(void) {
    xm_u32_t max;
    xm_s32_t part, i;

    max = ~0;
    part = -1;
    for (i=0; i<noPartitions; ++i) {
        if (partitionTab[i].schedData->policy==SCHED_CLASS_PRIORITY) {
            if ((partitionTab[i].status & PARTITION_ACTIVE)) {
                if (partitionTab[i].schedData->priority < max) {
                    part = i;
                    max = partitionTab[i].schedData->priority;
                }
            }
        }
    }

    return part;
}

xm_s32_t SchedBandwidthNext(void) {
    xm_u64_t ratio, max;
    xm_s32_t i, part;

    part = -1;
    max = 0;
    for (i=0; i<noPartitions; ++i) {
        if (partitionTab[i].schedData->policy==SCHED_CLASS_BANDWIDTH) {
            if ((partitionTab[i].status & PARTITION_ACTIVE) && partitionTab[i].header->busy) {
                ratio = (partitionTab[i].schedData->bw.budget * partitionTab[i].schedData->bw.ratio) / partitionTab[i].schedData->bw.run;
                if (ratio > max) {
                    max = ratio;
                    part = i;
                }
            }
        }
    }

    return part;
}

void SchedBandwidthInit(void) {
    xm_u32_t i;
    xmTime_t now;

    XM_get_time(XM_EXEC_CLOCK, &now);
    for (i=0; i<noPartitions; ++i) {
        partitionTab[i].schedData->bw.budget = now;
        partitionTab[i].schedData->bw.ratio = 1;
        partitionTab[i].schedData->bw.run = 1;
    }
}

xm_s32_t SchedIdleNext(void) {
    xm_u32_t origIdle;

    origIdle = nextIdle;
    nextIdle = (nextIdle + 1) % noPartitions;
    while (!(partitionTab[nextIdle].status & PARTITION_ACTIVE)) {
        nextIdle = (nextIdle + 1) % noPartitions;
        if (nextIdle == origIdle) {
            return -1;
        }
    }

    return nextIdle;
}

void SchedIdleInit(void) {
    nextIdle = 0;
}

struct Scheduler scheduler[SCHED_CLASS_MAX] = {
        [SCHED_CLASS_PRIORITY] = { "Priority", NULL, SchedPriorityNext },
        [SCHED_CLASS_BANDWIDTH] = { "Bandwidth", SchedBandwidthInit, SchedBandwidthNext },
        [SCHED_CLASS_IDLE] = { "Idle", SchedIdleInit, SchedIdleNext },
};

void *AllocMem(xmSize_t size) {
    void *ret;

    ret = NULL;
    if (poolEnd - poolPtr >= size) {
        ret = (void *)poolPtr;
        poolPtr += size;
    }
    memset(ret, 0, size);

    return ret;
}

void CheckPartitions(void) {
    struct xmcMemoryArea memMap[libXmParams.partInfTab->noPhysicalMemoryAreas];
    struct SpareHeader *header;
    xm_u32_t e, i;

    e = XM_get_physmem_map(memMap, libXmParams.partInfTab->noPhysicalMemoryAreas);
    for (i=0; i<e; ++i) {
        if (memMap[i].flags & SPARE_MEM_FLAG) {
            header = (struct SpareHeader *)memMap[i].startAddr;
            if (header->magic) {
                if (header->partitionId >= 0 && header->partitionId < e) {
                    partitionTab[header->partitionId].status = PARTITION_ACTIVE;
                    partitionTab[header->partitionId].header = header;
                }
            }
        }
    }
}

void NewSlot(trapCtxt_t *ctxt) {
    XM_get_time(XM_HW_CLOCK, &slotStart);
    CheckPartitions();
    newSlot = 1;
    pSlotEnd = slotEnd;
    slotEnd = slotStart + libXmParams.partCtrlTab->schedInfo.slotDuration;
}

void InitMem(void) {
    struct xmcMemoryArea memMap[libXmParams.partInfTab->noPhysicalMemoryAreas];
    xm_u32_t e, i;

    poolPtr = (xmAddress_t)_eguest;

    e = XM_get_physmem_map(memMap, libXmParams.partInfTab->noPhysicalMemoryAreas);
    if (e != libXmParams.partInfTab->noPhysicalMemoryAreas) {
        printf("Error retrieving physical memory map\n");
        printf("Unable to run Spare Host. Halting\n");
        XM_halt_partition(XM_PARTITION_SELF);
    }
    for (i=0; i<e; ++i) {
        if (memMap[i].flags & SPARE_MEM_FLAG) {
            noPartitions++;
        } else {
            if ((poolPtr > memMap[i].startAddr) && poolPtr <= (memMap[i].startAddr + memMap[i].size)) {
                poolEnd = memMap[i].startAddr + memMap[i].size;
            }
        }
    }
}

void InitPartitions(void) {
    xm_s32_t i;

    partitionTab = AllocMem(sizeof(struct Partition) * noPartitions);
    for (i=0; i<noPartitions; ++i) {
        partitionTab[i].schedData = &inputSchedData[i];
    }
}

void InitSpareHost(void) {
    xm_s32_t i;

    cPart = -1;
    InitMem();
    InitPartitions();
    for (i=0; i<SCHED_CLASS_MAX; ++i) {
        if (scheduler[i].init) {
            scheduler[i].init();
        }
    }
    InstallTrapHandler(XAL_XMEXT_TRAP(XM_VT_EXT_CYCLIC_SLOT_START), NewSlot);
    XM_unmask_irq(XM_VT_EXT_FIRST+XM_VT_EXT_CYCLIC_SLOT_START);
    XM_enable_irqs();
}

void PartitionMain(void) {
    xm_s32_t i;
    xmTime_t guestStart, guestStop;

    InitSpareHost();
    CheckPartitions();

    while (1) {
        for (i=0; i<SCHED_CLASS_MAX; ++i) {
            if (scheduler[i].next) {
                cPart = scheduler[i].next();
                if (cPart >= 0) {
                    cSched = i;
                    XM_set_spare_guest(cPart, &guestStart, &guestStop);
                    if (guestStart < slotStart) {
                        guestStop = pSlotEnd;
                    }
                    break;
                }
            }
        }
        if (cPart < 0) {
            XM_idle_self();
        } else if (cSched == SCHED_CLASS_BANDWIDTH) {
            partitionTab[cPart].schedData->bw.run += guestStop - guestStart;
        }
    }
}
