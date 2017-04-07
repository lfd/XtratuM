/*
 * $FILE: xmconf.h
 *
 * Config parameters for both, XM and partitions
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XMCONF_H_
#define _XMCONF_H_

#ifndef _XM_KERNEL_
#include <xm_inc/arch/arch_types.h>
#include <xm_inc/config.h>
#include <xm_inc/arch/xm_def.h>
#include <xm_inc/arch/xmconf.h>
#include <xm_inc/devid.h>
#else
#include <arch/xmconf.h>
#include <devid.h>
#endif

typedef struct {
    xm_u32_t id:16, subId:16;
} xmDev_t;

struct xmcHmSlot {
    xm_u32_t action:31, log:1;
  
// Logging
#define XM_HM_LOG_DISABLED 0
#define XM_HM_LOG_ENABLED 1

// Actions
//@% <track id="hm-action-list"> 
#define XM_HM_AC_IGNORE 0
#define XM_HM_AC_SHUTDOWN 1
#define XM_HM_AC_COLD_RESET 2
#define XM_HM_AC_WARM_RESET 3
#define XM_HM_AC_SUSPEND 4
#define XM_HM_AC_HALT 5
#define XM_HM_AC_PROPAGATE 6
//@% </track id="hm-action-list"> */
};

// Events
//@% <track id="hm-ev-xm-triggered">
#define XM_HM_EV_INTERNAL_ERROR 0
#define XM_HM_EV_UNEXPECTED_TRAP 1
#define XM_HM_EV_PARTITION_UNRECOVERABLE 2
#define XM_HM_EV_PARTITION_ERROR 3
#define XM_HM_EV_PARTITION_INTEGRITY 4
#define XM_HM_EV_MEM_PROTECTION 5
//@ \void{<track id="test-HM-overrun">}
#define XM_HM_EV_OVERRUN 6
//@  \void{</track id="test-HM-overrun">}
#define XM_HM_EV_SCHED_ERROR 7
#define XM_HM_EV_WATCHDOG_TIMER 8
//@ \void{<track id="test-HM-incompatibility">}
#define XM_HM_EV_INCOMPATIBLE_INTERFACE 9
//@  \void{</track id="test-HM-incompatibility">}

//@ </track id="hm-ev-xm-triggered">
#define XM_HM_MAX_GENERIC_EVENTS 10

#if defined(CONFIG_IA32)
struct xmcIoPort {
    xm_u32_t map[2048];
};
#else
#error Architecture unkown
#endif

struct xmcCommPort {
    xm_u32_t nameOffset;
    xm_s32_t channelId;
#define XM_NULL_CHANNEL -1
    xm_s32_t direction;
#define XM_SOURCE_PORT 0x2
#define XM_DESTINATION_PORT 0x1
    xm_s32_t type;
#define XM_SAMPLING_PORT 0
#define XM_QUEUING_PORT 1
};

//@% <track id="sched-cyclic-slot"> 
struct xmcSchedCyclicSlot {
    xm_u32_t id;
    xmId_t partitionId;
    xm_u32_t sExec; // offset (usec)
    xm_u32_t eExec; // offset+duration (usec)
};
//@% </track id="sched-cyclic-slot"> 

struct xmcSchedCyclicPlan {
    xmId_t id;   
    xm_u32_t majorFrame; // in useconds
    xm_s32_t noSlots;
    xmAddress_t slotsOffset;
};

struct xmcSchedCyclic {  
    //xm_s32_t noPlans;
    //struct xmcSchedCyclicPlan planTab[CONFIG_MAX_CYCLIC_PLANS];
    xm_u32_t schedCyclicPlansOffset;
    xm_s32_t noSchedCyclicPlans;
};

union xmcSchedParams {
    struct xmcSchedCyclic cyclic;
};

//@% <track id="doc-xmc-memory-area">
struct xmcMemoryArea {
    xmAddress_t startAddr;
    xmSize_t size;
#define XM_MEM_AREA_SHARED (1<<0)
#define XM_MEM_AREA_MAPPED (1<<1)
#define XM_MEM_AREA_WRITE (1<<2)
#define XM_MEM_AREA_ROM (1<<3)
#define XM_MEM_AREA_FLAG0 (1<<4)
#define XM_MEM_AREA_FLAG1 (1<<5)
#define XM_MEM_AREA_FLAG2 (1<<6)
#define XM_MEM_AREA_FLAG3 (1<<7)
    xm_u32_t flags;
    xmAddress_t memoryRegionOffset;
};
//@% </track id="doc-xmc-memory-area">

struct xmcRsw {
    xm_s32_t noPhysicalMemoryAreas;    
    xm_u32_t physicalMemoryAreasOffset;
    xmAddress_t entryPoint;
};

struct xmcTrace {
    xmDev_t dev;
    xm_u32_t bitmap;
};

struct xmcPartition {
    xmId_t id;
    xm_u32_t imageId;
    xm_u32_t nameOffset;
    xm_u32_t flags;
#define XM_PART_CPU_MASK 0xff
#define XM_PART_SUPERVISOR 0x100
#define XM_PART_BOOT 0x200
#define XM_PART_FP 0x400
#ifdef CONFIG_SPARE_SCHEDULING
#define XM_PART_SPARE_G 0x800
#define XM_PART_SPARE_H 0x1000
#endif
    xmAddress_t loadPhysAddr;
    xmAddress_t headerOffset;
    xm_s32_t noPhysicalMemoryAreas;
    xm_u32_t physicalMemoryAreasOffset;
    xmDev_t consoleDev;
    struct xmcTemporalRestrictions {
        xm_u32_t period;
        xm_u32_t duration;
    } temporalRestrictions;
    struct xmcPartitionArch arch;
    xm_u32_t commPortsOffset;
    xm_s32_t noPorts;
    struct xmcHmSlot hmTab[XM_HM_MAX_EVENTS];
    xm_u32_t ioPortsOffset;
    xm_s32_t noIoPorts;
    struct xmcTrace trace;
};

/* <track id="test-channel-struct"> */
struct xmcCommChannel {
#define XM_SAMPLING_CHANNEL 0
#define XM_QUEUING_CHANNEL 1
    xm_s32_t type;
  
    union {
        struct {
            xm_s32_t maxLength;
            xm_s32_t maxNoMsgs;
        } q;
        struct {
            xm_s32_t maxLength;
        } s;
    };
    xm_u32_t validPeriod;
};
/* </track id="test-channel-struct"> */

struct xmcMemoryRegion {
    xmAddress_t startAddr;
    xmSize_t size;
#define XMC_REG_FLAG_PGTAB (1<<0)
    xm_u32_t flags;
};

struct xmcHwIrq {
    xm_s32_t owner;
#define XM_IRQ_NO_OWNER -1
};

struct xmcHpv {
    xmAddress_t loadPhysAddr;
    xm_s32_t noPhysicalMemoryAreas;
    xm_u32_t physicalMemoryAreasOffset;
    xm_s32_t noCpus;
    struct cpu {
        xmId_t id;
        xm_u32_t features; // Enable/disable features
        xm_u32_t freq; // KHz
#define XM_CPUFREQ_AUTO 0
        xm_u32_t schedPolicy;
#define XM_SCHED_CYCLIC 0
#define XM_SCHED_RR 1
#define XM_SCHED_RM 2
        union xmcSchedParams schedParams;
    } cpuTab[CONFIG_NO_CPUS];
    struct xmcHpvArch arch;
    struct xmcHmSlot hmTab[XM_HM_MAX_EVENTS];
    xmDev_t hmDev;
    xmDev_t consoleDev;
    struct xmcHwIrq hwIrqTab[CONFIG_NO_HWIRQS];  
    struct xmcTrace trace;
};

#ifdef CONFIG_DEV_MEMBLOCK
struct xmcMemBlock {
    xmAddress_t startAddr;
    xm_s32_t size;
};
#endif

struct xmcDevice {
#ifdef CONFIG_DEV_MEMBLOCK
    xmAddress_t memBlocksOffset;
    xm_s32_t noMemBlocks;
#endif
#ifdef CONFIG_DEV_PC_UART
    struct xmcPcUartCfg {
   
    } pcUart;
#endif
#ifdef CONFIG_DEV_PC_VGA
    struct xmcPcVgaCfg {
   
    } pcVga;
#endif
};

#define XMC_VERSION 2
#define XMC_SUBVERSION 0
#define XMC_REVISION 0

struct //__attribute__ ((__packed__)) 
xmc {
    xm_u32_t checksum;
#define XMC_SIGNATURE 0x24584d43 // $XMC
    xm_u32_t signature;
    xmSize_t size;
// Reserved(8).VERSION(8).SUBVERSION(8).REVISION(8)
#define XMC_SET_VERSION(_ver, _subver, _rev) ((((_ver)&0xFF)<<16)|(((_subver)&0xFF)<<8)|((_rev)&0xFF))
#define XMC_GET_VERSION(_v) (((_v)>>16)&0xFF)
#define XMC_GET_SUBVERSION(_v) (((_v)>>8)&0xFF)
#define XMC_GET_REVISION(_v) ((_v)&0xFF)
    xm_u32_t version;
    xm_u32_t fileVersion;
    xm_u32_t nameOffset;
    struct xmcHpv hpv;
    struct xmcRsw rsw;
    xmAddress_t partitionTabOffset;
    xm_s32_t noPartitions;
    xmAddress_t memoryRegionsOffset;
    xm_u32_t noRegions;    
    xmAddress_t schedCyclicSlotsOffset;
    xm_s32_t noSchedCyclicSlots;
    xmAddress_t schedCyclicPlansOffset;
    xm_s32_t noSchedCyclicPlans;
    xmAddress_t commChannelTabOffset;
    xm_s32_t noCommChannels;
    xmAddress_t physicalMemoryAreasOffset;
    xm_s32_t noPhysicalMemoryAreas;
    xmAddress_t commPortsOffset;
    xm_s32_t noCommPorts;
    xmAddress_t ioPortsOffset;
    xm_s32_t noIoPorts;
    xmAddress_t stringsOffset;
    xm_s32_t stringTabLength;
    struct xmcDevice deviceTab;
    
};

#ifdef _XM_KERNEL_
extern const struct xmc xmcTab;
extern struct xmcPartition *xmcPartitionTab;
extern struct xmcMemoryRegion *xmcMemRegTab;
extern struct xmcCommChannel *xmcCommChannelTab;
extern struct xmcMemoryArea *xmcPhysMemAreaTab;
extern struct xmcCommPort *xmcCommPorts;
extern struct xmcIoPort *xmcIoPortTab;
extern struct xmcSchedCyclicSlot *xmcSchedCyclicSlotTab;
extern struct xmcSchedCyclicPlan *xmcSchedCyclicPlanTab;
#ifdef CONFIG_SPARE_SCHEDULING
extern struct xmcSchedCyclicSlot *xmcSchedSpareSlotTab;
#endif
extern char *xmcStringTab;
#ifdef CONFIG_DEV_MEMBLOCK
struct xmcMemBlock *xmcMemBlockTab;
#endif
#endif

#endif
