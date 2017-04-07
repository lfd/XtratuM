/*
 * $FILE: irqs.c
 *
 * Independent part of interrupt handling
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <boot.h>
#include <irqs.h>
#include <kthread.h>
#include <sched.h>
#include <stdc.h>
#include <processor.h>
#include <objects/hm.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif

// Definitions
struct irqTabEntry irqHandlerTab[HWIRQ_NR];
trapHandler_t trapHandlerTab[TRAP_NR];
hwIrqCtrl_t hwIrqCtrl[HWIRQ_NR];

static xm_s32_t DefaultIrqHandler(irqCtxt_t *ctxt, void *data) {
    kprintf("Unexpected irq %d\n", ctxt->irqNr);
    return 0;
}

static xm_s32_t UnexpectedTrapHandler(irqCtxt_t *ctxt) {
    return (ctxt->irqNr+XM_HM_MAX_GENERIC_EVENTS);
}

static xm_s32_t TriggerIrqHandler(irqCtxt_t *ctxt, void *data) {
    SetHwIrqPending(partitionTab[xmcTab.hpv.hwIrqTab[ctxt->irqNr].owner], ctxt->irqNr);   
    return 0;
}

void SetTrapPending(irqCtxt_t *ctxt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    
    ASSERT(!(XMAtomicGet(&sched->cKThread->ctrl.g->partitionControlTable->iFlags)&IFLAGS_TRAP_PEND_MASK));
    XMAtomicSetMask(IFLAGS_TRAP_PEND_MASK, &sched->cKThread->ctrl.g->partitionControlTable->iFlags);
}

void DoTrap(irqCtxt_t *ctxt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmHmLog_t hmLog;
    xm_s32_t action;
    ASSERT(ctxt->irqNr<TRAP_NR);

    if (trapHandlerTab[ctxt->irqNr])
        if (trapHandlerTab[ctxt->irqNr](ctxt))
            return;

    memset(&hmLog, 0, sizeof(xmHmLog_t));
    CpuCtxt2HmCpuCtxt(ctxt, &hmLog.cpuCtxt);
    hmLog.eventId=ctxt->irqNr+XM_HM_MAX_GENERIC_EVENTS;
    if (IsSvIrqCtxt(ctxt)) {
        hmLog.system=1;
        if (sched->cKThread->ctrl.g)
            hmLog.partitionId=sched->cKThread->ctrl.g->cfg->id;
        else
            hmLog.partitionId=XM_HYPERVISOR_ID;
    } else {
        hmLog.partitionId=sched->cKThread->ctrl.g->cfg->id;
    }
#ifdef CONFIG_VERBOSE_TRAP
    DumpState(1, ctxt);
#endif
    action=HmRaiseEvent(&hmLog);
    if (IsSvIrqCtxt(ctxt) && !hmLog.system)
        PartitionPanic(0, 0, "Partition in unrecoverable state\n");

    if (!IsSvIrqCtxt(ctxt)) {
        if (action)
            SetTrapPending(ctxt);
    } else {
        SystemPanic(IRQ_PANIC, ctxt, "Unexpected/unhandled trap - TRAP: 0x%x ERROR CODE: 0x%x\n", sched->cKThread->ctrl.g->partitionControlTable->trap2Vector[ctxt->irqNr], GET_ECODE(ctxt));
    }
}

void DoIrq(irqCtxt_t *ctxt) {
    localCpu_t *cpu = GET_LOCAL_CPU();
    ASSERT(!HwIsSti());
#ifdef CONFIG_OBJ_STATUS_ACC
    systemStatus.noIrqs++;
#endif
    cpu->irqNestingCounter++;
    HwAckIrq(ctxt->irqNr);
    if (irqHandlerTab[ctxt->irqNr].handler)
    (*(irqHandlerTab[ctxt->irqNr].handler))(ctxt, irqHandlerTab[ctxt->irqNr].data);
    else
    DefaultIrqHandler(ctxt, 0);

    //HwEndIrq(ctxt->irqNr);
    cpu->irqNestingCounter--;
    do {
        if (cpu->irqNestingCounter==SCHED_PENDING)
        Scheduling();
    }while(cpu->irqNestingCounter==SCHED_PENDING);
    ASSERT(!HwIsSti());
    ASSERT(!(cpu->irqNestingCounter&SCHED_PENDING));
}

void __VBOOT SetupIrqs(void) {
    xm_s32_t irqNr;

    for (irqNr=0; irqNr<HWIRQ_NR; irqNr++) {
        if (xmcTab.hpv.hwIrqTab[irqNr].owner!=XM_IRQ_NO_OWNER) {
            SetIrqHandler(irqNr, TriggerIrqHandler, 0);
        } else  {
            SetIrqHandler(irqNr, DefaultIrqHandler, 0);
        }
    }

    for (irqNr=0; irqNr<TRAP_NR; irqNr++)
        trapHandlerTab[irqNr]=0;

    ArchSetupIrqs();
}

irqHandler_t SetIrqHandler(xm_s32_t irq, irqHandler_t irqHandler, void *data) {
    irqHandler_t old_handler=irqHandlerTab[irq].handler;

    if (irqHandler) {
	irqHandlerTab[irq]=(struct irqTabEntry){
	    .handler=irqHandler,
	    .data=data,
	};
    } else
	irqHandlerTab[irq]=(struct irqTabEntry){
	    .handler=DefaultIrqHandler,
	    .data=0,
	};
    return old_handler;
}

trapHandler_t SetTrapHandler(xm_s32_t trap, trapHandler_t trapHandler) {
    trapHandler_t old_handler=trapHandlerTab[trap];

    if (trapHandler)
        trapHandlerTab[trap]=trapHandler;
    else
        trapHandlerTab[trap]=UnexpectedTrapHandler;

    return old_handler;
}
