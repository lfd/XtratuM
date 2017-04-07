/*
 * $FILE: irqs.c
 *
 * IRQS' code
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
#include <bitwise.h>
#include <irqs.h>
#include <kthread.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <arch/segments.h>
#include <arch/pic.h>

static xm_s32_t Ia32TrapPageFault(irqCtxt_t *ctxt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t cr2;
    SaveCr2(cr2);
#if 0
    {
	unsigned long pgd, *vPgd, pgt, *vPgt, p;
	struct physPage *page;
	SavePgd(pgd);
	kprintf("pgd: %x\n", pgd);
	if (!(page=PmmFindPage(pgd)))
	    while (1) kprintf("Error PGD\n");
	vPgd=(xm_u32_t *)VCacheMapPage(pgd, page);
	kprintf("vpgd: %x\n", vPgd);
	pgt=vPgd[VA2Pgd(cr2)]&PAGE_MASK;
	kprintf("pgd: %x\n", pgd);
	if (!(page=PmmFindPage(pgt)))
	    while (1) kprintf("Error PGT\n");
	vPgt=(xm_u32_t *)VCacheMapPage(pgt, page);
	p=vPgt[VA2Pgt(cr2)]&PAGE_MASK;
	kprintf("PAGE: %x\n", p);
	if (!(page=PmmFindPage(p)))
	    while (1) kprintf("Error PGT\n");
	kprintf("TYPE: %d\n", page->type);
	}
#endif

    sched->cKThread->ctrl.g->partitionControlTable->arch.cr2=cr2;
    if (!IsSvIrqCtxt(ctxt)) {
        if ((cr2<CONFIG_XM_OFFSET)||(cr2==(~0))) {
            SetTrapPending(ctxt);
            return 1;
        }
    }
    return 0;
}

static xm_s32_t Ia32GenProFault(irqCtxt_t *ctxt) {
    if (!IsSvIrqCtxt(ctxt)) {
        SetTrapPending(ctxt);
        return 1;
    }
    return 0;
}

static xm_s32_t Ia32DeviceNotAvailableFault(irqCtxt_t *ctxt) {
    if (!IsSvIrqCtxt(ctxt)) {
        SetTrapPending(ctxt);
        return 1;
    }
    return 0;
}

void ArchSetupIrqs(void) {
    extern void (*hwIrqHndlTab[0])(void);
    extern void (*trapHndlTab[0])(void);
    extern void UnexpectedIrq(void);
    extern void HypercallHandler(void);
    xm_s32_t irqNr;

    /* Setting up the HW irqs */
    for (irqNr=0; irqNr<HWIRQ_NR; irqNr++) {
	ASSERT(hwIrqHndlTab[irqNr]);
	HwSetIrqGate(irqNr+FIRST_EXTERNAL_VECTOR, hwIrqHndlTab[irqNr]);
    }
    
    /*
      XXX:  the  single  difference  between   a   trap_gate and    an
      interrupt_gate is  that the IF flags is  unmodified in  the first
      one whereas it is cleaned in the second one
    */
    HwSetTrapGate(0, trapHndlTab[0]);
    HwSetIrqGate(1, trapHndlTab[1]);
    HwSetIrqGate(2, trapHndlTab[2]);
    HwSetSysGate(3, trapHndlTab[3]);
    HwSetSysGate(4, trapHndlTab[4]);
    HwSetSysGate(5, trapHndlTab[5]);
    HwSetTrapGate(6, trapHndlTab[6]);
    HwSetTrapGate(7, trapHndlTab[7]);
    HwSetTrapGate(8, trapHndlTab[8]);
    HwSetTrapGate(9, trapHndlTab[9]);
    HwSetTrapGate(10, trapHndlTab[10]);
    HwSetTrapGate(11, trapHndlTab[11]);
    HwSetTrapGate(12, trapHndlTab[12]);
    //HwSetTrapGate(13, trapHndlTab[13]);
    HwSetIrqGate(13, trapHndlTab[13]);
    HwSetIrqGate(14, trapHndlTab[14]);
    HwSetTrapGate(15, trapHndlTab[15]);
    HwSetTrapGate(16, trapHndlTab[16]);
    HwSetTrapGate(17, trapHndlTab[17]);
    HwSetTrapGate(18, trapHndlTab[18]);
    HwSetTrapGate(19, trapHndlTab[19]);
    //HwSetCallGate(earlyXmGdt, XM_HYPERCALL_CALLGATE_SEL,
    //HypercallHandler, 2, 1, XM_CS);

    SetTrapHandler(13, Ia32GenProFault);
    SetTrapHandler(14, Ia32TrapPageFault);
    SetTrapHandler(7, Ia32DeviceNotAvailableFault);

    InitPic(0x20, 0x28);
}

// returns -1 when no irq is pending
// otherwise returns the pending irq

static inline xm_s32_t AreIrqsPending(kThread_t *k) {    
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t pendingIrq;

    if(k==sched->idleKThread)
	return -1;

    if (XMAtomicGet(&k->ctrl.g->partitionControlTable->iFlags)&IFLAGS_IRQ_MASK) {
	pendingIrq=XMAtomicGet(&k->ctrl.g->partitionControlTable->hwIrqsPend)&~XMAtomicGet(&k->ctrl.g->partitionControlTable->hwIrqsMask);
	if (pendingIrq) {
	    pendingIrq=_Ffs(pendingIrq);
	    //ASSERT(pendingIrq>=0&&pendingIrq<16);
	    return pendingIrq;
	}       
    }
    return -1;
}

static inline xm_s32_t AreExtIrqsPending(kThread_t *k) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t eirq;

    if(k==sched->idleKThread)
	return -1;

    eirq=XMAtomicGet(&k->ctrl.g->partitionControlTable->extIrqsPend)&~XMAtomicGet(&k->ctrl.g->partitionControlTable->extIrqsMask);
    if (eirq) {
	eirq=_Ffs(eirq);
	return eirq;
    }       
  
    return -1;
}

static inline xm_s32_t AreExtTrapsPending(kThread_t *k) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t eirq;

    if(k==sched->idleKThread)
	return -1;

    eirq=XMAtomicGet(&k->ctrl.g->partitionControlTable->extIrqsPend)&XM_EXT_TRAPS;
    if (eirq) {
	eirq=_Ffs(eirq);
	return eirq;
    }       
  
    return -1;
}

static inline void Irq2Eip(int irqNr, xm_u16_t *xcs, xm_u32_t *eip){
     localSched_t *sched=GET_LOCAL_SCHED();
     genericDesc_t *idtTab;

     idtTab=(genericDesc_t *)sched->cKThread->ctrl.g->partitionControlTable->arch.idtr.linearBase;

     *xcs=idtTab[irqNr].high>>16;
     *eip=(idtTab[irqNr].high&0xFFFF)|(idtTab[irqNr].low&0xFFFF0000);
     if ((*eip>=XM_OFFSET)||!(*xcs&0x3))
	 PartitionPanic(0, 0, "[Irq2Eip] error emulating IRQ(%d) [0x%x:0x%x]", irqNr, *xcs, *eip);
     
     if (!(idtTab[irqNr].low&(1<<8)))
	 XMAtomicClearMask(IFLAGS_IRQ_MASK, &sched->cKThread->ctrl.g->partitionControlTable->iFlags);
 }

static inline void __FixStackPc(irqCtxt_t *ctxt, int irqNr, int errCode) {
#define ERRCODE_TAB 0x27d00UL
    xm_u32_t eip, *esp=0, iFlags, needErrCode=(irqNr<32)?((ERRCODE_TAB&(1<<irqNr))?1:0):0;
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u16_t xcs;

    iFlags=XMAtomicGet(&sched->cKThread->ctrl.g->partitionControlTable->iFlags);
    iFlags=(iFlags&IFLAGS_MASK);
    
    if ((irqNr<TRAP_NR)&&sched->cKThread->ctrl.g->overrideTrapTab[irqNr].cs) {
	xcs=sched->cKThread->ctrl.g->overrideTrapTab[irqNr].cs;
	eip=sched->cKThread->ctrl.g->overrideTrapTab[irqNr].eip;
    } else {
	if (irqNr>=(sched->cKThread->ctrl.g->partitionControlTable->arch.idtr.limit+1)/sizeof(gateDesc_t))
	    PartitionPanic(0, ctxt, "[__FixStackPc] bad irq (%d)", irqNr);
	Irq2Eip(irqNr, &xcs, &eip);
    }

    if ((ctxt->xcs&0x3)==(xcs&0x3)) {
	esp=(xm_u32_t *)ctxt->esp;
    } else {
	xm_u32_t ss=0;
	switch(xcs&0x3) {
	case 0x1:
	    ss=sched->cKThread->ctrl.g->kArch.tss.ss1;
	    esp=(xm_u32_t *)sched->cKThread->ctrl.g->kArch.tss.esp1;
	    break;
	case 0x2:
	    ss=sched->cKThread->ctrl.g->kArch.tss.ss2;
	    esp=(xm_u32_t *)sched->cKThread->ctrl.g->kArch.tss.esp2;
	    break;
	default:
	    PartitionPanic(0, 0, "[__FixStackPc] CS (0x%x) corrupted", xcs);
	    }
	if (!ss||!esp)
	    PartitionPanic(0, 0, "[__FixStackPc] SS:ESP (0x%x:0x%x) invalid",ss,esp);
	
	*(--esp)=ctxt->xss;
	*(--esp)=ctxt->esp;
	ctxt->xss=ss;
    }

    /* Detect eary recursive Trap (Page Fault) otherwise the Partition esp will
     * go down until it hits a RO page (PGD, PGT,...) producing an XM panic.
     */
    if(ctxt->eip == eip)
	    PartitionPanic(0, 0, "[__FixStackPc] bad eip (0x%x)", eip);
    
    *(--esp)=ctxt->eflags;
    *(--esp)=ctxt->xcs;
    *(--esp)=ctxt->eip;
    /* IFLAGS must be poped from the stack before executing IRET */
    *(--esp)=iFlags;
    
    if (needErrCode)
	*(--esp)=errCode;
    ctxt->esp=(xm_u32_t)esp;
    ctxt->xcs=xcs;
    ctxt->eip=eip;
}

void __EmulIrq(irqCtxt_t *ctxt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t eIrq;

    if (!sched->cKThread->ctrl.g ||!(ctxt->xcs&0x3))
	return;
    
    // Do pending traps
    if (XMAtomicGet(&sched->cKThread->ctrl.g->partitionControlTable->iFlags)&IFLAGS_TRAP_PEND_MASK) {
	XMAtomicClearMask(IFLAGS_TRAP_PEND_MASK, &sched->cKThread->ctrl.g->partitionControlTable->iFlags);
	__FixStackPc(ctxt, sched->cKThread->ctrl.g->partitionControlTable->trap2Vector[ctxt->irqNr], ctxt->errorCode);
	return;
    }

    // pending hwirqs
    if (XMAtomicGet(&sched->cKThread->ctrl.g->partitionControlTable->iFlags)&IFLAGS_ATOMIC_MASK) {
	if ((ctxt->eip>=sched->cKThread->ctrl.g->partitionControlTable->arch.atomicArea.sAddr)&&(ctxt->eip<sched->cKThread->ctrl.g->partitionControlTable->arch.atomicArea.eAddr))
	    return;
    }

    if ((eIrq=AreIrqsPending(sched->cKThread))>-1) {
	ASSERT(eIrq<HWIRQ_NR);
	XMAtomicClearMask((1<<eIrq), &sched->cKThread->ctrl.g->partitionControlTable->hwIrqsPend);
	XMAtomicSetMask((1<<eIrq), &sched->cKThread->ctrl.g->partitionControlTable->hwIrqsMask);
	__FixStackPc(ctxt, sched->cKThread->ctrl.g->partitionControlTable->hwIrq2Vector[eIrq], 0);
	return;
    }

    // pending extirqs
    if ((XMAtomicGet(&sched->cKThread->ctrl.g->partitionControlTable->iFlags)&IFLAGS_IRQ_MASK)&&((eIrq=AreExtIrqsPending(sched->cKThread))>-1)) {
	XMAtomicClearMask((1<<eIrq), &sched->cKThread->ctrl.g->partitionControlTable->extIrqsPend);
	XMAtomicSetMask((1<<eIrq), &sched->cKThread->ctrl.g->partitionControlTable->extIrqsMask);
	__FixStackPc(ctxt, sched->cKThread->ctrl.g->partitionControlTable->extIrq2Vector[eIrq], 0);
	return;
    }

    // No emulation required
}
