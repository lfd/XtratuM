/*
 * $FILE: hypercalls.c
 *
 * XM's hypercalls
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
#include <kthread.h>
#include <gaccess.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <hypercalls.h>
#include <virtmm.h>
#include <vmmap.h>

__hypercall xm_s32_t UpdatePage32Sys(hypercallCtxt_t *ctxt, xmAddress_t pAddr, xm_u32_t val) {
    extern xm_s32_t (*UpdatePPag32HndlTab[NR_PPAG])(hypercallCtxt_t *, struct physPage *, xmAddress_t, xm_u32_t *);
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *page;
    xm_u32_t addr;

    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());

    if (!(page=PmmFindPage(pAddr, sched->cKThread, 0))) {
	kprintf("Page 0x%x not found\n", pAddr);
	return XM_INVALID_PARAM;
    }

    if (UpdatePPag32HndlTab[page->type])
	if (UpdatePPag32HndlTab[page->type](ctxt, page, pAddr, &val)<0)
	    return XM_INVALID_PARAM;

    addr=(xm_u32_t)VCacheMapPage(pAddr, page);
    *(xm_u32_t *)addr=val;
    VCacheUnlockPage(page);

    return XM_OK;
}

__hypercall xm_s32_t SetPageTypeSys(hypercallCtxt_t *ctxt, xmAddress_t pAddr, xm_u32_t type) {
    extern void (*UnsetPPagTypeHndlTab[NR_PPAG])(hypercallCtxt_t *, xmAddress_t, struct physPage *);
    extern void (*SetPPagTypeHndlTab[NR_PPAG])(hypercallCtxt_t *, xmAddress_t, struct physPage *);
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *page;
    CHECK_SHMAGIC(sched->cKThread);
    ASSERT(!HwIsSti());
    
    if (type>=NR_PPAG) {
	kprintf("Page type %d unkown\n", type);
	return XM_INVALID_PARAM;
    }

    if (!(page=PmmFindPage(pAddr, sched->cKThread, 0))) {
	kprintf("Page 0x%x not found\n", pAddr);
	return XM_INVALID_PARAM;
    }

    if (page->counter) {
	kprintf("Page 0x%x operation not allowed (counter: %d)\n", pAddr, page->counter);
	return XM_OP_NOT_ALLOWED;
    }
    //if (type!=page->type) {
	if (UnsetPPagTypeHndlTab[page->type])
	    UnsetPPagTypeHndlTab[page->type](ctxt, pAddr, page);
	
	if (SetPPagTypeHndlTab[type])
	    SetPPagTypeHndlTab[type](ctxt, pAddr, page);
	
	page->type=type;
//    }
    return XM_OK;

}
