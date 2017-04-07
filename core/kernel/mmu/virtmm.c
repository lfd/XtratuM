/*
 * $FILE: virtmm.c
 *
 * Virtual memory manager
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
#include <brk.h>
#include <boot.h>
#include <processor.h>
#include <virtmm.h>
#include <vmmap.h>
#include <stdc.h>
#include <arch/paging.h>
#include <arch/xm_def.h>

static xmAddress_t vmmStartAddr; 
static xm_s32_t noFrames;

xmAddress_t VmmAlloc(xm_s32_t nPag) {
    xmAddress_t vAddr;
    if ((noFrames-nPag)<0) 
	return 0;
    vAddr=vmmStartAddr;
    vmmStartAddr+=(nPag<<PAGE_SHIFT);
    noFrames-=nPag;
    ASSERT(noFrames>=0);
    return vAddr;
}

xm_s32_t VmmGetNoFreeFrames(void) {
    ASSERT(noFrames>=0);
    return noFrames;
}

void __VBOOT SetupVirtMM(xmAddress_t sAddr, xmAddress_t eAddr) {
    SetupVmMap();

    sAddr=ROUNDUP2PAGE(sAddr, PAGE_SIZE);
    eAddr=ROUNDUP2PAGE(eAddr, PAGE_SIZE);
    ASSERT((eAddr-sAddr)<=XM_VMAPSIZE);
    vmmStartAddr=sAddr;
    noFrames=(eAddr-sAddr)/PAGE_SIZE;
    kprintf("[VMM] Virtual memory map [0x%x-0x%x] %d pages\n", sAddr, eAddr, (eAddr-sAddr)/PAGE_SIZE);
}
