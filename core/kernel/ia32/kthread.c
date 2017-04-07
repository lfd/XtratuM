/*
 * $FILE: kthread.c
 *
 * Kernel, Guest or L0 context (ARCH dependent part)
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
    if (new->ctrl.g) {
        LoadGdt(new->ctrl.g->kArch.gdtr);
        LoadIdt(new->ctrl.g->kArch.idtr);
        LoadPgd(new->ctrl.g->kArch.pgd);
        TssClearBusy(&new->ctrl.g->kArch.gdtr, TSS_SEL);
        LoadTr(TSS_SEL);
    }
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

    k->ctrl.g->kArch.gdtr.limit=(sizeof(gdtDesc_t)*(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES))-1;
    k->ctrl.g->kArch.gdtr.linearBase=(xm_u32_t)k->ctrl.g->kArch.gdtTab;

    k->ctrl.g->kArch.cr0=DEFAULT_CR0;
    
    memcpy(k->ctrl.g->kArch.idtTab, xmIdt, sizeof(genericDesc_t)*IDT_ENTRIES);

    k->ctrl.g->kArch.idtr.limit=(sizeof(genericDesc_t)*IDT_ENTRIES)-1;
    k->ctrl.g->kArch.idtr.linearBase=(xm_u32_t)k->ctrl.g->kArch.idtTab;

    if (k->ctrl.g->cfg->noIoPorts>0) {
        memcpy(k->ctrl.g->kArch.tss.ioMap, xmcIoPortTab[k->ctrl.g->cfg->ioPortsOffset].map, 2048*sizeof(xm_u32_t));
        EnableTssIoMap(&k->ctrl.g->kArch.tss);
    } else
        DisableTssIoMap(&k->ctrl.g->kArch.tss);
    LoadTssDesc((desc_t *)&k->ctrl.g->kArch.gdtTab[TSS_SEL>>3], &k->ctrl.g->kArch.tss);
}

void KThreadArchInit(kThread_t *k) {
    __asm__ __volatile__ ("finit\n\t" ::);
    k->ctrl.g->kArch.tss.t.ss0=XM_DS;
    k->ctrl.g->kArch.tss.t.esp0=(xmAddress_t)&k->kStack[CONFIG_KSTACK_SIZE-4];
    SetWp();
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
	k->ctrl.g->partitionControlTable->extIrq2Vector[e]=e+0x90;
}

void FillArchPartitionInfTab(kThread_t *k, partitionInformationTable_t *partitionInfTab) {
}
