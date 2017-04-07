/*
 * $FILE: guest.h
 *
 * Guest shared info
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_GUEST_H_
#define _XM_GUEST_H_

#define XM_HYPERVISOR_ID 0xFF
#define XM_SYSTEM_ID XM_HYPERVISOR_ID
#define KTHREAD_MAGIC 0xA5A55A5A
#define PCT_SV_FLAG (1<<0)
#define IFLAGS_IRQ_BIT 0
#define IFLAGS_TRAP_PEND_BIT 1

#define IFLAGS_ARCH_BIT 16

#define IFLAGS_IRQ_MASK (1<<(IFLAGS_IRQ_BIT))
#define IFLAGS_TRAP_PEND_MASK (1<<(IFLAGS_TRAP_PEND_BIT))
#define IFLAGS_ARCH_MASK (0xFF<<IFLAGS_ARCH_BIT)

#define IFLAGS_MASK (IFLAGS_IRQ_MASK|IFLAGS_ARCH_MASK)
/* <track id="xm-interrupt-list"> */
#define XM_VT_HW_FIRST             (0)
#define XM_VT_HW_LAST              (31)
#define XM_VT_HW_MAX               (32)

#define XM_VT_HW_INTERNAL_BUS_TRAP_NR (1)
#define XM_VT_HW_UART2_TRAP_NR        (2)
#define XM_VT_HW_UART1_TRAP_NR        (3)
#define XM_VT_HW_IO_IRQ0_TRAP_NR      (4)
#define XM_VT_HW_IO_IRQ1_TRAP_NR      (5)
#define XM_VT_HW_IO_IRQ2_TRAP_NR      (6)
#define XM_VT_HW_IO_IRQ3_TRAP_NR      (7)
#define XM_VT_HW_TIMER1_TRAP_NR       (8)
#define XM_VT_HW_TIMER2_TRAP_NR       (9)
#define XM_VT_HW_DSU_TRAP_NR          (11)
#define XM_VT_HW_PCI_TRAP_NR          (14)

#define XM_VT_EXT_FIRST            (32)
#define XM_VT_EXT_MAX              (32)
#define XM_VT_EXT_LAST             (XM_VT_EXT_FIRST+XM_VT_EXT_MAX-1)

#define XM_VT_EXT_HW_TIMER          (0+XM_VT_EXT_FIRST)
#define XM_VT_EXT_EXEC_TIMER        (1+XM_VT_EXT_FIRST)
#define XM_VT_EXT_WATCHDOG_TIMER    (2+XM_VT_EXT_FIRST)
#define XM_VT_EXT_SHUTDOWN          (3+XM_VT_EXT_FIRST)
#define XM_VT_EXT_OBJDESC           (4+XM_VT_EXT_FIRST)

#define XM_VT_EXT_CYCLIC_SLOT_START (8+XM_VT_EXT_FIRST)

#define XM_VT_EXT_MEM_PROTECT       (16+XM_VT_EXT_FIRST)

/* <track id="xm-ipvi-list"> */
/* Inter-Partition Virtual Interrupts */
#define XM_MAX_IPVI 8

#define XM_VT_EXT_IPVI0            (24+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI1            (25+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI2            (26+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI3            (27+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI4            (28+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI5            (29+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI6            (30+XM_VT_EXT_FIRST)
#define XM_VT_EXT_IPVI7            (31+XM_VT_EXT_FIRST)
/* </track id="xm-ipvi-list"> */

/* </track id="xm-interrupt-list"> */

#define XM_EXT_TRAPS ((1<<(XM_VT_EXT_MEM_PROTECT-XM_VT_EXT_FIRST)))

#ifndef __ASSEMBLY__

#ifdef _XM_KERNEL_
#include <arch/atomic.h>
#include <arch/guest.h>
#include <arch/irqs.h>
#include <xmconf.h>
#include <objects/hm.h>
#else
#include <xm_inc/arch/atomic.h>
#include <xm_inc/arch/guest.h>
#include <xm_inc/arch/irqs.h>
#include <xm_inc/xmconf.h>
#include <xm_inc/objects/hm.h>
#endif

/* <track id="doc-Partition-Control-Table"> */
typedef struct {
    xm_u32_t magic;  
    xmAtomic_t iFlags;
    // BIT: 23..16: ARCH
    //      1: TRAP PENDING
    //      0: IRQ
    xmAtomic_t hwIrqsPend; // pending hw irqs
    xmAtomic_t hwIrqsMask; // masked hw irqs
    
    xmAtomic_t extIrqsPend; // pending extended irqs
    xmAtomic_t extIrqsMask; // masked extended irqs

    xmAtomic_t objDescClassPend; // Object descritors 

    struct pctArch arch;
    struct {
	xm_u32_t noSlot:16, reserved:16;
	xm_u32_t id;
	xm_u32_t slotDuration;
	xm_u32_t slotUsed;
	xm_u32_t slotAccum;
    } schedInfo;
    xm_u8_t trap2Vector[TRAP_NR];
    xm_u8_t hwIrq2Vector[HWIRQ_NR];
    xm_u16_t extIrq2Vector[XM_VT_EXT_MAX];
} partitionControlTable_t;
/* </track id="doc-Partition-Control-Table">  */

/* <track id="doc-Partition-Info-Table"> */
typedef struct {
    xm_u32_t signature;
#define PARTITION_INFORMATION_TABLE_SIGNATURE 0x584d5354 /* "XMST" */
    xm_u32_t xmVersion; // XM version
    xm_u32_t xmAbiVersion; // XM's abi version
    xm_u32_t xmApiVersion; // XM's api version
    xm_u32_t checksum;
    xm_u32_t resetCounter;
    xm_u32_t resetStatus;
    xm_u32_t cpuKhz;
    //////////////
    xmId_t id;
    char name[CONFIG_ID_STRING_LENGTH];
    xm_u32_t flags;
    xm_u32_t hwIrqs; // Hw interrupts belonging to the partition
    xm_s32_t noPartitions;
    xm_s32_t noPhysicalMemoryAreas;
    pitArch_t arch;
} partitionInformationTable_t;
/*  </track id="doc-Partition-Info-Table"> */

/*  <track id="checksum-fnt"> */
static inline xm_u32_t xmCalcChecksum(void *s, xm_u32_t size) {
/*  </track id="checksum-fnt"> */
    xm_u32_t c;
    xm_u8_t *e;
  
    for (c=0, e=s; e<((xm_u8_t *)s+size); e+=sizeof(xm_u8_t)) {
	c+=*e;
    }

    return ((~0UL)-c+1);
}

#define CHECK_SHMAGIC(_k) do { \
    if ((_k)->ctrl.g->partitionControlTable->magic!=KTHREAD_MAGIC) { \
	xmHmLog_t hmLog; \
	hmLog.partitionId=sched->cKThread->ctrl.g->cfg->id; \
	hmLog.system=0; \
	hmLog.eventId=XM_HM_EV_PARTITION_INTEGRITY; \
	HmRaiseEvent(&hmLog); \
	SystemPanic(0, 0, "[HYPERCALLS] partition control table tainted (MN 0x%x)", (_k)->ctrl.g->partitionControlTable->magic); \
    } \
} while(0)

#endif

#endif
