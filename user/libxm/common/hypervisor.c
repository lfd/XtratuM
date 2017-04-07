/*
 * $FILE: hypervisor.c
 *
 * Hypervisor related functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <hypervisor.h>
#include <xmhypercalls.h>
#include <arch/atomic_ops.h>

#include <xm_inc/bitwise.h>
#include <xm_inc/linkage.h>
#include <xm_inc/hypercalls.h>
#include <xm_inc/objdir.h>
#include <xm_inc/objects/mem.h>

__stdcall xm_s32_t XM_write_console(char *buffer, xm_s32_t length) {
    return XM_write_object(OBJDESC_BUILD(OBJ_CLASS_CONSOLE, XM_PARTITION_SELF, 0), buffer, length, 0);
}

__stdcall xm_s32_t XM_memory_copy(xmId_t dstId, xm_u32_t dstAddr, xmId_t srcId, xm_u32_t srcAddr, xm_u32_t size) {

    if (srcId==XM_PARTITION_SELF)
	return XM_write_object(OBJDESC_BUILD(OBJ_CLASS_MEM, dstId, 0), (void *)dstAddr, size, (void *)srcAddr);
    
    if (dstId==XM_PARTITION_SELF)
	return XM_read_object(OBJDESC_BUILD(OBJ_CLASS_MEM, srcId, 0), (void *)srcAddr, size, (void *)dstAddr);
    
    return XM_INVALID_PARAM;
}

__stdcall xm_s32_t XM_get_physmem_map(struct xmcMemoryArea *memMap, xm_s32_t noAreas) {
    union memCmd args;
    
    args.physMemMap.areas=memMap;
    args.physMemMap.noAreas=noAreas;
    
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_MEM, XM_PARTITION_SELF, 0), XM_MEM_GET_PHYSMEMMAP, &args);
}

// XXX: iFlags is not taken into account
__stdcall xm_s32_t XM_exec_pendirqs(void *ctxt) {
    xm_u32_t ipend, irq;

    if (!(XMAtomicGet(&XM_params_get_PCT()->iFlags)&IFLAGS_IRQ_MASK))
	return 0;

    ipend=XMAtomicGet(&XM_params_get_PCT()->hwIrqsPend)&~XMAtomicGet(&XM_params_get_PCT()->hwIrqsMask);
    if (ipend) {
	irq=_Ffs(ipend);
	XMAtomicClearMask(IFLAGS_IRQ_MASK, &XM_params_get_PCT()->iFlags);
	XMAtomicClearMask((1<<irq), &XM_params_get_PCT()->hwIrqsPend);
	XMAtomicSetMask((1<<irq), &XM_params_get_PCT()->hwIrqsMask);
	if (!ctxt)
	    __XM_partition_emul_hwIrq(irq);	
	else
	    __XM_partition_emul_hwIrqCtxt(irq, ctxt);

	return 1;
    }

    ipend=XMAtomicGet(&XM_params_get_PCT()->extIrqsPend)&~XMAtomicGet(&XM_params_get_PCT()->extIrqsMask);
    if (ipend) {
	irq=_Ffs(ipend);
	XMAtomicClearMask(IFLAGS_IRQ_MASK, &XM_params_get_PCT()->iFlags);
	XMAtomicClearMask((1<<irq), &XM_params_get_PCT()->extIrqsPend);
	XMAtomicSetMask((1<<irq), &XM_params_get_PCT()->extIrqsMask);
	if (!ctxt)
	    __XM_partition_emul_extIrq(irq);
	else
	    __XM_partition_emul_extIrqCtxt(irq, ctxt);

	return 1;
    }
    return 0;
}

/* <track id="disable-irqs-imp"> */
__stdcall void XM_disable_irqs(void) {
    XMAtomicClearMask(IFLAGS_IRQ_MASK, &XM_params_get_PCT()->iFlags);
}

/* </track id="disable-irqs-imp"> */
__stdcall void XM_enable_irqs(void) {
    XMAtomicSetMask(IFLAGS_IRQ_MASK, &XM_params_get_PCT()->iFlags);
    XM_exec_pendirqs(0);
}

__stdcall xm_s32_t XM_are_irqs_enabled(void) {
    return (XMAtomicGet(&XM_params_get_PCT()->iFlags)&IFLAGS_IRQ_MASK)?1:0;
}

__stdcall xm_s32_t XM_mask_irq(xm_u32_t noIrq){
    if (noIrq>=XM_VT_HW_FIRST&&noIrq<=XM_VT_HW_LAST) {
	if ((1<<noIrq)&XM_params_get_PIT()->hwIrqs)
	    XM_mask_hwirq(noIrq);
	
	XMAtomicSetMask((1<<noIrq), &XM_params_get_PCT()->hwIrqsMask);
	return XM_OK;
    } else if (noIrq>=XM_VT_EXT_FIRST&&noIrq<=XM_VT_EXT_LAST) {
	noIrq-=XM_VT_EXT_FIRST;
	XMAtomicSetMask((1<<noIrq), &XM_params_get_PCT()->extIrqsMask);
	return XM_OK;
    }
    return XM_INVALID_PARAM;
}

__stdcall xm_s32_t XM_unmask_irq(xm_u32_t noIrq) {
    if (noIrq>=XM_VT_HW_FIRST&&noIrq<=XM_VT_HW_LAST) {
	if ((1<<noIrq)&XM_params_get_PIT()->hwIrqs)
	    XM_unmask_hwirq(noIrq);
	XMAtomicClearMask((1<<noIrq), &XM_params_get_PCT()->hwIrqsMask);	
	if (XMAtomicGet(&XM_params_get_PCT()->iFlags)&IFLAGS_IRQ_MASK)
	    XM_exec_pendirqs(0);
	return XM_OK;
    } else if (noIrq>=XM_VT_EXT_FIRST&&noIrq<=XM_VT_EXT_LAST) {
	noIrq-=XM_VT_EXT_FIRST;
	XMAtomicClearMask((1<<noIrq), &XM_params_get_PCT()->extIrqsMask);
	if (XMAtomicGet(&XM_params_get_PCT()->iFlags)&IFLAGS_IRQ_MASK)
	    XM_exec_pendirqs(0);
	return XM_OK;
    }
    return XM_INVALID_PARAM;
}

