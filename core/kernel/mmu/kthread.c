/*
 * $FILE: kthread.c
 *
 * Kernel and Guest context
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
#include <kthread.h>
#include <physmm.h>
#include <sched.h>
#include <stdc.h>
#include <virtmm.h>
#include <vmmap.h>
#include <xmef.h>
#include <arch/xm_def.h>

void BuildPartitionMemoryImg(kThread_t *k, struct xmPartitionHdr *xmPartitionHdr) {
    if (!(k->ctrl.g->kArch.pgd=SetupPartitionPgd(k, xmPartitionHdr)))
	PartitionPanic(0, 0,"[BuildPartitionMemoryImg] Unable to build a valid memory img");
}

partitionControlTable_t *MapPartitionCtrlTab(xm_u32_t partitionCtrlTab) {
    xm_u32_t vAddr, e;
    // Missed to unmap the previous existing one
    if (partitionCtrlTab&(PAGE_SIZE-1))
	PartitionPanic(0, 0, "PartitionControlTable must be aligned to PAGE\n");
    vAddr=VmmAlloc(SIZE2PAGES(sizeof(partitionControlTable_t)));
    for (e=0; e<sizeof(partitionControlTable_t); e+=PAGE_SIZE)
	VmMapPage(partitionCtrlTab+e, vAddr+e, _PG_PRESENT|_PG_RW);
    
    return (partitionControlTable_t *)vAddr;
}

void UnmapPartitionCtrlTab(partitionControlTable_t *partitionCtrlTab) {
    //xm_u32_t vAddr, e;
    // vAddr=(xm_u32_t)partitionCtrlTab;
    // if (vAddr<XM_START_VMMAP||vAddr>XM_END_VMMAP) return;
    //for (e=0; e<sizeof(partitionControlTable_t); e+=PAGE_SIZE)
//	VmUnmapPage(vAddr+e);
    //  VmmFree(vAddr, SIZE2PAGES(sizeof(partitionControlTable_t)));
}

partitionInformationTable_t *MapPartitionInfTab(xm_u32_t partitionInfTab) {
    xm_u32_t vAddr, e;
    if (partitionInfTab&(PAGE_SIZE-1))
	PartitionPanic(0, 0, "PartitionInformationTable must be aligned to PAGE\n");
    vAddr=VmmAlloc(SIZE2PAGES(sizeof(partitionInformationTable_t)));
    
    for (e=0; e<sizeof(partitionInformationTable_t); e+=PAGE_SIZE)
	VmMapPage(partitionInfTab+e, vAddr+e, _PG_PRESENT|_PG_RW);

    return (partitionInformationTable_t *)vAddr;
}

void UnmapPartitionInfTab(partitionInformationTable_t *partitionInfTab) {
    //xm_u32_t vAddr, e;
    // vAddr=(xm_u32_t)partitionInfTab;
    //ASSERT((vAddr>=XM_START_VMMAP)&&(vAddr<XM_END_VMMAP));
    //for (e=0; e<sizeof(partitionInformationTable_t); e+=PAGE_SIZE)
//	VmUnmapPage(vAddr+e);
    //    VmmFree(vAddr, SIZE2PAGES(sizeof(partitionInformationTable_t)));
}
