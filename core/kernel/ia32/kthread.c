/*
 * $FILE: kthread.c
 *
 * Kernel, Guest or L0 context (ARCH dependent part)
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <brk.h>
#include <gaccess.h>
#include <kthread.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <arch/xm_def.h>

void SwitchKThreadArchPre(kThread_t *new, kThread_t *current) {
    if (current->ctrl.g) {
	SavePgd(current->ctrl.g->kArch.pgd);
    }
    if(new->ctrl.g) {
	LoadGdt(new->ctrl.g->kArch.gdtr);
	LoadIdt(new->ctrl.g->kArch.idtr);
	xmTss[GET_CPU_ID()].t.ss1=new->ctrl.g->kArch.tss.ss1;
	xmTss[GET_CPU_ID()].t.esp1=new->ctrl.g->kArch.tss.esp1;
	xmTss[GET_CPU_ID()].t.ss2=new->ctrl.g->kArch.tss.ss2;
	xmTss[GET_CPU_ID()].t.esp2=new->ctrl.g->kArch.tss.esp2;

	if (new->ctrl.g->cfg->noIoPorts>0) {
	    memcpy(xmTss[GET_CPU_ID()].ioMap, xmcIoPortTab[new->ctrl.g->cfg->ioPortsOffset].map, 2048*sizeof(xm_u32_t));
	    EnableTssIoMap(&xmTss[GET_CPU_ID()]);
	} else
	    DisableTssIoMap(&xmTss[GET_CPU_ID()]);
	
	LoadPgd(new->ctrl.g->kArch.pgd);
    }
    //xmTss[GET_CPU_ID()].t.ss0=XM_DS;
    xmTss[GET_CPU_ID()].t.esp0=(xm_u32_t)&new->kStack[CONFIG_KSTACK_SIZE-4];
    LoadCr0(DEFAULT_CR0);
}

void SwitchKThreadArchPost(kThread_t *current) {
    if (current->ctrl.g) {
	LoadCr0(current->ctrl.g->kArch.cr0);
    }
}

void ArchCreatePartition(kThread_t *k) {
    ASSERT(k->ctrl.g);
    memcpy((xm_u8_t *)k->ctrl.g->kArch.gdtTab, (xm_u8_t *)xmGdt, sizeof(gdtDesc_t)*(XM_GDT_ENTRIES+CONFIG_PARTITION_NO_GDT_ENTRIES));

    k->ctrl.g->kArch.gdtr=(pseudoDesc_t){
	.limit=(sizeof(gdtDesc_t)*(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES))-1,
	.linearBase=(xm_u32_t)k->ctrl.g->kArch.gdtTab,
    };

    k->ctrl.g->kArch.cr0=DEFAULT_CR0;
    
    memcpy(k->ctrl.g->kArch.idtTab, xmIdt, sizeof(genericDesc_t)*IDT_ENTRIES);
    k->ctrl.g->kArch.idtr=(pseudoDesc_t){
	.limit=(sizeof(genericDesc_t)*IDT_ENTRIES)-1,
	.linearBase=(xm_u32_t)k->ctrl.g->kArch.idtTab,
    };    
}

void FillArchPartitionCtrlTab(kThread_t *k, partitionControlTable_t *partitionCtrlTab) {
    xm_s32_t e;
    partitionCtrlTab->arch.cr3=k->ctrl.g->kArch.pgd;
    partitionCtrlTab->arch.cr0=DEFAULT_CR0;
    XMAtomicSet(&k->ctrl.g->partitionControlTable->hwIrqsMask, ~0);
    XMAtomicClearMask(IFLAGS_IRQ_MASK, &k->ctrl.g->partitionControlTable->iFlags);
    for (e=0; e<TRAP_NR; e++)
	k->ctrl.g->partitionControlTable->trap2Vector[e]=e;

    for (e=0; e<HWIRQ_NR; e++)
	k->ctrl.g->partitionControlTable->hwIrq2Vector[e]=e+0x20;
    
    for (e=0; e<XM_VT_EXT_MAX; e++)
	k->ctrl.g->partitionControlTable->extIrq2Vector[e]=0x90+e;
}

void FillArchPartitionInfTab(kThread_t *k, partitionInformationTable_t *partitionInfTab) {
}
