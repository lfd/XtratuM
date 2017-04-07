/*
 * $FILE: panic.c
 *
 * Code executed in a panic situation
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */
/*
 * Changes: on 28 Feb, 2012 by Salva Peiro, <speiro@ai2.upv.es>:
 *          Replace Fatal() and Panic with SystemPanic(), PartitionPanic();
 */

#include <kthread.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <objects/hm.h>


extern xm_s8_t localInfoInit;

#ifdef CONFIG_DEBUG
static void StackBackTrace(xm_u32_t ebp){
    xm_s32_t e=1;
    kprintf("Stack backtrace:\n   ", 0);
    while (ebp) {
	kprintf("[0x%x] ", *(xm_u32_t *)(ebp+4));
	if (!(e%5)) kprintf("\n   ");
	ebp=*(xm_u32_t *)ebp;
	e++;
    }
    kprintf("\n");
}
#endif

void DumpState(xm_s8_t from, void *iregs) {
    irqCtxt_t *regs = iregs;
    xm_u32_t ebp;
    xm_u16_t xcs;

    (localInfoInit)?kprintf("CPU state:\n"):kprintf("CPU state:\n");

    if (from) {
	irqCtxt_t *iregs=(irqCtxt_t*)regs;
	xm_u32_t cr2;
	SaveCr2(cr2);
	kprintf("EIP: 0x%x:[<0x%x>]", (xm_u32_t) 0xffff&iregs->xcs, (xm_u32_t) iregs->eip);
	if (iregs->xcs&0x3)
	    kprintf(" ESP: 0x%x:[<0x%x>]", (xm_u32_t) 0xffff&iregs->xss, (xm_u32_t) iregs->esp);
	else {
	    xm_u32_t cesp;
	    xm_u16_t css;
	    HwSaveStack(cesp);
	    SaveSs(css);
	    kprintf(" ESP: 0x%x:[<0x%x>]", (xm_u32_t) 0xffff&css, (xm_u32_t)cesp);
	}
	kprintf(" EFLAGS: 0x%x  \n", (xm_u32_t) iregs->eflags);
	kprintf("EAX: 0x%x EBX: 0x%x ECX: 0x%x EDX: 0x%x\n", (xm_u32_t) iregs->eax, (xm_u32_t) iregs->ebx, (xm_u32_t) iregs->ecx, (xm_u32_t) iregs->edx);
	kprintf("ESI: 0x%x EDI: 0x%x EBP: 0x%x\n",(xm_u32_t) iregs->esi, (xm_u32_t) iregs->edi,	(xm_u32_t) iregs->ebp);
	kprintf("DS: 0x%x ES: 0x%x FS: 0x%x GS: 0x%x\n", (xm_u32_t) 0xffff & iregs->xds, (xm_u32_t) 0xffff & iregs->xes, (xm_u32_t) 0xffff & iregs->xfs, (xm_u32_t) 0xffff & iregs->xgs);
	kprintf("CR2: 0x%x\n", cr2);

	xcs=iregs->xcs;
	ebp=iregs->ebp;
    } else {
	hypercallCtxt_t *sregs=(hypercallCtxt_t*)regs;
	kprintf("EIP: 0x%x:[<0x%x>]", (xm_u32_t) 0xffff&sregs->xcs, (xm_u32_t) sregs->eip);
	if (sregs->xcs&0x3)
	    kprintf(" ESP: 0x%x:[<0x%x>]", (xm_u32_t) 0xffff&sregs->xss, (xm_u32_t)sregs->esp);
	else {
	    xm_u32_t cesp;
	    xm_u16_t css;
	    HwSaveStack(cesp);
	    SaveSs(css);
	    kprintf(" ESP: 0x%x:[<0x%x>]", (xm_u32_t) 0xffff&css, (xm_u32_t)cesp);
	}
	kprintf(" EFLAGS: 0x%x  \n", (xm_u32_t) sregs->eflags);
	kprintf("EAX: 0x%x EBX: 0x%x ECX: 0x%x EDX: 0x%x\n", (xm_u32_t) sregs->eax, (xm_u32_t) sregs->ebx, (xm_u32_t) sregs->ecx, (xm_u32_t) sregs->edx);
	kprintf("ESI: 0x%x EDI: 0x%x EBP: 0x%x\n", (xm_u32_t) sregs->esi, (xm_u32_t) sregs->edi, (xm_u32_t) sregs->ebp);
	kprintf("DS: 0x%x ES: 0x%x FS: 0x%x GS: 0x%x\n", (xm_u32_t) 0xffff & sregs->xds, (xm_u32_t) 0xffff & sregs->xes, (xm_u32_t) 0xffff & sregs->xfs, (xm_u32_t) 0xffff & sregs->xgs);
	xcs=sregs->xcs;
	ebp=sregs->ebp;
    }

#ifdef CONFIG_DEBUG
    if (regs&&!(xcs&0x3))
	StackBackTrace(ebp);
    else {
	SaveEbp(ebp);
	StackBackTrace(ebp);
    }
#endif
}

void SystemPanic(xm_s8_t from, void *regs, xm_s8_t *fmt, ...) {
    localSched_t *sched=GET_LOCAL_SCHED();
    irqCtxt_t *ctxt=regs;
    xmHmLog_t hmLog;
    va_list vl;
    ASSERT(fmt);

    HwCli();

    kprintf("System FATAL ERROR:\n");
    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
    kprintf("\n");

    if (sched&&sched->cKThread&&ctxt)
        DumpState(from, ctxt);
/*#ifdef CONFIG_DEBUG
    else {
        xmWord_t ebp=SaveBp();
        StackBackTrace(ebp);
    }
#endif*/

    hmLog.system=1;
    hmLog.eventId=XM_HM_EV_INTERNAL_ERROR;
    if (sched&&sched->cKThread&&sched->cKThread->ctrl.g)
        hmLog.partitionId=sched->cKThread->ctrl.g->cfg->id;
    memset(hmLog.word, 0, sizeof(hmLog.word));

    HmRaiseEvent(&hmLog);

    HaltSystem();
}

void PartitionPanic(xm_s8_t from, void *regs, xm_s8_t *fmt, ...) {
    localSched_t *sched=GET_LOCAL_SCHED();
    irqCtxt_t *ctxt=regs;
    xmHmLog_t hmLog;
    va_list vl;
    ASSERT(fmt);

    kprintf("System PANIC [");

    if (sched->cKThread->ctrl.g)
	kprintf("0x%x:id(%d)]:\n", sched->cKThread, sched->cKThread->ctrl.g->cfg->id);
    else
	kprintf("0x%x:IDLE]:\n", sched->cKThread);

    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
    kprintf("\n");
    if (ctxt)
	DumpState(from, ctxt);
/*
#ifdef CONFIG_DEBUG
    else {
	xmWord_t ebp=SaveBp();
	StackBackTrace(ebp);
    }
#endif
*/

    if (sched->cKThread==sched->idleKThread)
        HaltSystem();

    hmLog.system=0;
    hmLog.eventId=XM_HM_EV_PARTITION_ERROR;
    if (sched&&sched->cKThread&&sched->cKThread->ctrl.g)
        hmLog.partitionId=sched->cKThread->ctrl.g->cfg->id;
    memset(hmLog.word, 0, sizeof(hmLog.word));

    HmRaiseEvent(&hmLog);

    // Finish current kthread
    SET_KTHREAD_FLAG(sched->cKThread, KTHREAD_HALTED_F);
    if (sched->cKThread==sched->idleKThread)
	SystemPanic(from, regs, "Idle thread triggered a PANIC???");
    Scheduling();
    
    SystemPanic(from, regs, "[PANIC] This line should not be reached!!!");
}
