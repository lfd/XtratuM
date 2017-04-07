/*
 * $FILE: physmm.c
 *
 * Physical memory manager
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

static struct dynList cacheLRU;
static struct physPage **physPageTab;

struct physPage *PmmFindPage(xmAddress_t pAddr, kThread_t *k, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;    
    struct xmcPartition *cfg;
    xm_s32_t e;

    pAddr&=PAGE_MASK;
    cfg=k->ctrl.g->cfg;
    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
	memArea=&xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset];
	if ((pAddr>=memArea->startAddr)&&((pAddr+PAGE_SIZE-1)<=(memArea->startAddr+memArea->size-1))) {
	    if (!(xmcMemRegTab[memArea->memoryRegionOffset].flags&XMC_REG_FLAG_PGTAB)) return 0;
	    if (flags)
		*flags=memArea->flags;
	    return &physPageTab[memArea->memoryRegionOffset][(pAddr-xmcMemRegTab[memArea->memoryRegionOffset].startAddr)>>PAGE_SHIFT];
	}
    }
    kprintf("Page 0x%x does not belong to %d\n", pAddr, cfg->id);

    return 0;
}

xm_s32_t PmmFindAddr(xmAddress_t pAddr, kThread_t *k, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;    
    struct xmcPartition *cfg;
    xm_s32_t e;

    pAddr&=PAGE_MASK;
    cfg=k->ctrl.g->cfg;
    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
	memArea=&xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset];
	if ((pAddr>=memArea->startAddr)&&((pAddr+PAGE_SIZE-1)<=(memArea->startAddr+memArea->size-1))) {
	    if (flags)
		*flags=memArea->flags;
	    return 1;
	}
    }
    kprintf("Page 0x%x does not belong to %d\n", pAddr, cfg->id);

    return 0;
}

void *VCacheMapPage(xmAddress_t pAddr, struct physPage *page) {
    if (page->mapped)
	return (void *)(page->vAddr+(pAddr&(PAGE_SIZE-1)));
    
    if (VmmGetNoFreeFrames()<=0) {
	struct physPage *victimPage;
	// Unmapping the last mapped page
	victimPage=DynListRemoveTail(&cacheLRU);
	ASSERT(victimPage);
	victimPage->unlocked=0;
	victimPage->mapped=0;
	page->vAddr=victimPage->vAddr;
	victimPage->vAddr=~0;
    } else
	page->vAddr=VmmAlloc(1);

    page->mapped=1;
    VmMapPage(pAddr&PAGE_MASK, page->vAddr, _PG_PRESENT|_PG_RW);

    return (void *)(page->vAddr+(pAddr&(PAGE_SIZE-1)));
}

void VCacheUnlockPage(struct physPage *page) {
    ASSERT(page&&page->mapped);
    if (!page->unlocked) {
	page->unlocked=1;
	DynListInsertHead(&cacheLRU, &page->listNode);
    }
}

void ZeroPhysMM(void) {
    xm_s32_t e;

    for (e=0; e<xmcTab.noRegions; e++) {
        ASSERT(!(xmcMemRegTab[e].size&(PAGE_SIZE-1))&&!(xmcMemRegTab[e].startAddr&(PAGE_SIZE-1)));
        if (xmcMemRegTab[e].flags&XMC_REG_FLAG_PGTAB)
            memset(physPageTab[e], 0, sizeof(struct physPage)*(xmcMemRegTab[e].size/PAGE_SIZE));
    }
}

void SetupPhysMM(void) {
    xm_s32_t e;

    DynListInit(&cacheLRU);
    GET_MEMZ(physPageTab, sizeof(struct physPage *)*xmcTab.noRegions);

    for (e=0; e<xmcTab.noRegions; e++) {
	ASSERT(!(xmcMemRegTab[e].size&(PAGE_SIZE-1))&&!(xmcMemRegTab[e].startAddr&(PAGE_SIZE-1)));
	if (xmcMemRegTab[e].flags&XMC_REG_FLAG_PGTAB)
	    GET_MEM(physPageTab[e], sizeof(struct physPage)*(xmcMemRegTab[e].size/PAGE_SIZE));
	else
	    physPageTab[e]=0;
    }

}
