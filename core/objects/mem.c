/*
 * $FILE: mem.c
 *
 * System physical memory
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
#include <boot.h>
#include <hypercalls.h>
#include <objdir.h>
#include <physmm.h>
#include <sched.h>
#include <stdc.h>

#include <objects/mem.h>

static inline xm_s32_t CopyMem(void *dst, void *src, xm_s32_t size) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *pageSrc, *pageDst;
    xm_u8_t *vAddrSrc, *vAddrDst;
    xm_u32_t addrSrc, addrDst;
    xm_s32_t s;

    if (size<=0) return 0;
    for (addrDst=(xm_u32_t)dst, addrSrc=(xm_u32_t)src, s=size; addrSrc<((xm_u32_t)src+size); addrSrc+=PAGE_SIZE, addrDst+=PAGE_SIZE, s-=PAGE_SIZE) {
	pageSrc=PmmFindPage(addrSrc&PAGE_MASK, sched->cKThread, 0);
	pageDst=PmmFindPage(addrDst&PAGE_MASK, sched->cKThread, 0);	
	vAddrSrc=VCacheMapPage(addrSrc, pageSrc);
	vAddrDst=VCacheMapPage(addrDst, pageDst);
	memcpy(vAddrDst, vAddrSrc, (s<PAGE_SIZE)?s:PAGE_SIZE);
	VCacheUnlockPage(pageSrc);
	VCacheUnlockPage(pageDst);
    }
    return size;
}

static xm_s32_t ReadMem(xmObjDesc_t desc, xm_u8_t *dst, xmSize_t size, xm_u8_t *src) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmAddress_t sAddr, eAddr;
    xmId_t partId;
    xm_s32_t e;

    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;

    for (e=0; e<sched->cKThread->ctrl.g->cfg->noPhysicalMemoryAreas; e++) {
	sAddr=xmcPhysMemAreaTab[e+sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset].startAddr;
	eAddr=sAddr+xmcPhysMemAreaTab[e+sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset].size;
	if ((((xmAddress_t)dst)>=sAddr)&&((xmAddress_t)dst+size)<eAddr)
	    break;
    }

    if (e>=sched->cKThread->ctrl.g->cfg->noPhysicalMemoryAreas)
	return XM_INVALID_PARAM;

    for (e=0; e<xmcPartitionTab[partId].noPhysicalMemoryAreas; e++) {
	sAddr=xmcPhysMemAreaTab[e+xmcPartitionTab[partId].physicalMemoryAreasOffset].startAddr;
	eAddr=sAddr+xmcPhysMemAreaTab[e+xmcPartitionTab[partId].physicalMemoryAreasOffset].size;
	if ((((xmAddress_t)src)>=sAddr)&&((xmAddress_t)src+size)<eAddr)
	    break;
    }

    if (e>=xmcPartitionTab[partId].noPhysicalMemoryAreas)
	return XM_INVALID_PARAM;
    
    return CopyMem(dst, src, size);
}

static xm_s32_t WriteMem(xmObjDesc_t desc, xm_u8_t *src, xmSize_t size, xm_u8_t *dst) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmAddress_t sAddr, eAddr;
    xmId_t partId;
    xm_s32_t e;

    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;

    for (e=0; e<sched->cKThread->ctrl.g->cfg->noPhysicalMemoryAreas; e++) {
	sAddr=xmcPhysMemAreaTab[e+sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset].startAddr;
	eAddr=sAddr+xmcPhysMemAreaTab[e+sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset].size;
	if ((((xmAddress_t)src)>=sAddr)&&((xmAddress_t)src+size)<eAddr)
	    break;
    }

    if (e>=sched->cKThread->ctrl.g->cfg->noPhysicalMemoryAreas)
	return XM_INVALID_PARAM;

    for (e=0; e<xmcPartitionTab[partId].noPhysicalMemoryAreas; e++) {
	sAddr=xmcPhysMemAreaTab[e+xmcPartitionTab[partId].physicalMemoryAreasOffset].startAddr;
	eAddr=sAddr+xmcPhysMemAreaTab[e+xmcPartitionTab[partId].physicalMemoryAreasOffset].size;
	if ((((xmAddress_t)dst)>=sAddr)&&((xmAddress_t)dst+size)<eAddr)
	    break;
    }

    if (e>=xmcPartitionTab[partId].noPhysicalMemoryAreas)
	return XM_INVALID_PARAM;

    return CopyMem(dst, src, size);
}

static xm_s32_t CtrlMem(xmObjDesc_t desc, xm_u32_t cmd, union memCmd *__gParam args) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t min;
    if (!args)
	return XM_INVALID_PARAM;
    
    if (__CheckGParam(0, args, sizeof(union memCmd))<0) return XM_INVALID_PARAM;
    switch(cmd) {
    case XM_MEM_GET_PHYSMEMMAP:
	if (args->physMemMap.noAreas<0)
	    return XM_INVALID_PARAM;
	min=(args->physMemMap.noAreas<sched->cKThread->ctrl.g->cfg->noPhysicalMemoryAreas)?args->physMemMap.noAreas:sched->cKThread->ctrl.g->cfg->noPhysicalMemoryAreas;
	if (__CheckGParam(0, args->physMemMap.areas, min*sizeof(struct xmcMemoryArea))<0) return XM_INVALID_PARAM;
	
	memcpy(args->physMemMap.areas, &xmcPhysMemAreaTab[sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset], sizeof(struct xmcMemoryArea)*min);
	return min;
	break;
    }

    return XM_INVALID_PARAM;
}

static const struct object memObj={
    .Read=(readObjOp_t)ReadMem,
    .Write=(writeObjOp_t)WriteMem,
    .Ctrl=(ctrlObjOp_t)CtrlMem,
};

xm_s32_t __VBOOT SetupMem(void) {    
    objectTab[OBJ_CLASS_MEM]=&memObj;
    return 0;
}

REGISTER_OBJ(SetupMem);
