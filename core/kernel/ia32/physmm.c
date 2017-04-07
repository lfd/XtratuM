/*
 * $FILE: physmm.c
 *
 * Physical memory manager
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
#include <list.h>
#include <brk.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <virtmm.h>
#include <vmmap.h>
#include <arch/paging.h>
#include <arch/xm_def.h>

/*
static xm_s32_t UpdateStdPag32(hypercallCtxt_t *ctxt, struct physPage *pagePgd, xmAddress_t pAddr, xmAddress_t *val) {
    kprintf("Updating STD: %x\n", pAddr);
    return XM_OK;
    
    }*/

static xm_s32_t UpdatePgdPag32(hypercallCtxt_t *ctxt, struct physPage *pagePgd, xmAddress_t pAddr, xmAddress_t *val) {
    xm_u32_t vPgd, oldVal;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *pagePgt;

    if (((pAddr&(PAGE_SIZE-1))>>2)>=XM_PGD_START) {
	kprintf("[UpdPGD] Page offset (%d) > XM_PGD_START\n", (pAddr&(PAGE_SIZE-1))>>2);
	return XM_INVALID_PARAM;
    }
    vPgd=(xm_u32_t)VCacheMapPage(pAddr, pagePgd);
    oldVal=*(xm_u32_t *)vPgd;
    VCacheUnlockPage(pagePgd);

    if ((*val)&_PG_PRESENT) {
	if (!(pagePgt=PmmFindPage(*val, sched->cKThread, 0))) {
	    kprintf("[UpdPGD] Page (0x%x) does not belong to partition\n", (*val)&PAGE_MASK);
	    return XM_INVALID_PARAM;
	}

	if (pagePgt->type!=PPAG_PGT) {	    
	    kprintf("[UpdPGD] Page (0x%x) is not a PGT page\n", (*val)&PAGE_MASK);
	    return XM_INVALID_PARAM;
	}
		
	if ((*val)&_PG_PSE) {
	    kprintf("[UpdPGD] Page (0x%x) has PSE set\n", (*val)&PAGE_MASK);
	    return XM_INVALID_PARAM;
	}
	
	if ((*val)&_PG_GLOBAL)
	    (*val)&=~_PG_GLOBAL;

	PPAG_INC_COUNT(pagePgt);
    }

    if (oldVal&_PG_PRESENT) {
	if (!(pagePgt=PmmFindPage(oldVal, sched->cKThread, 0)))
	    return XM_INVALID_PARAM;
	ASSERT(pagePgt->type==PPAG_PGT);
	ASSERT(pagePgt->counter>0);
	PPAG_DEC_COUNT(pagePgt);
    }

    return XM_OK;
}

static xm_s32_t UpdatePgtPag32(hypercallCtxt_t *ctxt, struct physPage *pagePgt, xmAddress_t pAddr, xmAddress_t *val) {
    xm_u32_t vPgt, oldVal, areaFlags;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *page;
   
    vPgt=(xm_u32_t)VCacheMapPage(pAddr, pagePgt);

    oldVal=*(xm_u32_t *)vPgt;
    VCacheUnlockPage(pagePgt);

    if ((*val)&_PG_PRESENT) {
	if (!(page=PmmFindPage(*val, sched->cKThread, &areaFlags))) {
	    if (!PmmFindAddr(*val, sched->cKThread, &areaFlags)) {
		kprintf("[UpdPGT] Page (0x%x) does not belong to partition\n", (*val)&PAGE_MASK);
		return XM_INVALID_PARAM;
	    }
	}	
	if (page) {
	    if ((page->type!=PPAG_STD)||!(areaFlags&XM_MEM_AREA_WRITE))
		(*val)&=~_PG_RW;
	    
	    PPAG_INC_COUNT(page);
	} else {
	    if (!(areaFlags&XM_MEM_AREA_WRITE))
		(*val)&=~_PG_RW;
	}
    }
    if (oldVal&_PG_PRESENT) {	
	if (!(page=PmmFindPage(oldVal, sched->cKThread, 0))) {
	    if (!PmmFindAddr(oldVal, sched->cKThread, &areaFlags)) {
		kprintf("[UpdPGT] Page (0x%x) does not exist\n", (oldVal&PAGE_MASK));
		return XM_INVALID_PARAM;
	    }
	}
	if (page) {
	    ASSERT(page->counter>0);
	    PPAG_DEC_COUNT(page);
	}
    }

    return XM_OK;
}

xm_s32_t (*UpdatePPag32HndlTab[NR_PPAG])(hypercallCtxt_t *, struct physPage *, xmAddress_t, xmAddress_t *)={
    [PPAG_STD]=0, 
    [PPAG_PGD]=UpdatePgdPag32, 
    [PPAG_PGT]=UpdatePgtPag32,
};

static void SetPgd(hypercallCtxt_t *ctxt, xmAddress_t pAddr, struct physPage *pagePgd) {
    xm_u32_t *vPgd=VCacheMapPage(pAddr, pagePgd), e;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *pagePgt;

    for (e=0; e<XM_PGD_START; e++) {
	if (vPgd[e]&_PG_PRESENT) {
	    if (!(pagePgt=PmmFindPage(vPgd[e], sched->cKThread, 0))) {
		kprintf("[SetPgd] %d is not owner of 0x%x\n", sched->cKThread->ctrl.g->cfg->id, vPgd[e]&PAGE_MASK);
		return;
	    }

	    if (pagePgt->type!=PPAG_PGT) {
		kprintf("[SetPgd] %d (0x%x) is not a PGT\n", sched->cKThread->ctrl.g->cfg->id, vPgd[e]&PAGE_MASK, vPgd[e]&PAGE_MASK);
		vPgd[e]&=~_PG_PRESENT;
		continue;
	    }
	    
	    if (vPgd[e]&_PG_GLOBAL)
		vPgd[e]&=~_PG_GLOBAL;

	    if (vPgd[e]&_PG_PSE) {
		kprintf("[SetPgd] %d (0x%x) PSE set\n", sched->cKThread->ctrl.g->cfg->id, vPgd[e]&PAGE_MASK, vPgd[e]&PAGE_MASK);
		vPgd[e]&=~_PG_PRESENT;
		continue;	       
	    }
	    PPAG_INC_COUNT(pagePgt);
	}
    }
    memcpy((xm_u8_t *)&vPgd[XM_PGD_START], (xm_u8_t *)&xmPgd[XM_PGD_START], sizeof(xm_u32_t)*(XM_PGD_END-XM_PGD_START));
    VCacheUnlockPage(pagePgd);    
}

static void UnsetPgd(hypercallCtxt_t *ctxt, xmAddress_t pAddr, struct physPage *pagePgd) {
    xm_u32_t *vPgd=VCacheMapPage(pAddr, pagePgd);
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *pagePgt;
    xm_s32_t e;

    for (e=0; e<XM_PGD_START; e++) {
	if (vPgd[e]&_PG_PRESENT) {	    
	    if (!(pagePgt=PmmFindPage(vPgd[e], sched->cKThread, 0)))
		return;
	    ASSERT(pagePgt->type==PPAG_PGT);
	    ASSERT(pagePgt->counter>0);
	    PPAG_DEC_COUNT(pagePgt);
	}
    }
    VCacheUnlockPage(pagePgd);
}

static void SetPgt(hypercallCtxt_t *ctxt, xmAddress_t pAddr, struct physPage *pagePgt) {
    xm_u32_t *vPgt=VCacheMapPage(pAddr, pagePgt), areaFlags;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *page;
    xm_s32_t e;
    
    for (e=0; e<1024; e++) {
	if (vPgt[e]&_PG_PRESENT) {
	    if (!(page=PmmFindPage(vPgt[e], sched->cKThread, &areaFlags))) {
		kprintf("[SetPgt] %d is not owner of 0x%x\n", sched->cKThread->ctrl.g->cfg->id, vPgt[e]&PAGE_MASK);
		return;
	    }

	    if (vPgt[e]&_PG_GLOBAL)
		vPgt[e]&=~_PG_GLOBAL;

	    if ((page->type!=PPAG_STD)||!(areaFlags&XM_MEM_AREA_WRITE))
	    	vPgt[e]&=~_PG_RW;
	    
	    PPAG_INC_COUNT(page);
	}
    }
    VCacheUnlockPage(pagePgt);    
}

static void UnsetPgt(hypercallCtxt_t *ctxt, xmAddress_t pAddr, struct physPage *pagePgt) {
    xm_u32_t *vPgt=VCacheMapPage(pAddr, pagePgt), e;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *page;

    for (e=0; e<1024; e++) {
	if (vPgt[e]&_PG_PRESENT) {
	    if (!(page=PmmFindPage(vPgt[e], sched->cKThread, 0)))
		return;
	    PPAG_DEC_COUNT(page);
	}
    }
    VCacheUnlockPage(pagePgt);
}

void (*UnsetPPagTypeHndlTab[NR_PPAG])(hypercallCtxt_t *, xmAddress_t, struct physPage *)={
    [PPAG_STD]=0, 
    [PPAG_PGD]=UnsetPgd, 
    [PPAG_PGT]=UnsetPgt,
};

void (*SetPPagTypeHndlTab[NR_PPAG])(hypercallCtxt_t *, xmAddress_t, struct physPage *)={
    [PPAG_STD]=0, 
    [PPAG_PGD]=SetPgd, 
    [PPAG_PGT]=SetPgt,
};
