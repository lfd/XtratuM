/*
 * $FILE: vmmap.c
 *
 * Virtual memory map management
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
#include <kthread.h>
#include <stdc.h>
#include <vmmap.h>
#include <virtmm.h>
#include <physmm.h>
#include <arch/xm_def.h>
#include <arch/processor.h>

xm_u32_t SetupPartitionPgd(kThread_t *k, struct xmPartitionHdr *xmgHdr) {
    xm_u32_t cPgt, *vPgd, *vPgt, pPgt, noPgts, areaFlags;
    struct physPage *page;
    xmAddress_t pgd, pgts, vAddr, pAddr;
    xmSize_t pgSize;
    xm_s32_t e;

    pgSize=xmgHdr->pagTabSize;
    pgSize-=PAGE_SIZE;
    if (pgSize<0) {
        kprintf("[SetupPartitionPgd] Not enough memory to create a PGD\n");
        return 0;
    }

    pgd=xmgHdr->pagTabAddr;
    pgts=pgd+PAGE_SIZE;

    // Check pgd is valid
    if (!(page=PmmFindPage(pgd, k, 0))) {
        kprintf("[SetupPartitionPgd] Invalid Pgd 0x%x\n",  pgd);
        return 0;
    }

    ASSERT(page);
    page->type=PPAG_PGD;
    for (pAddr=pgts, noPgts=0; pgSize>=PAGE_SIZE; pAddr+=PAGE_SIZE, noPgts++, pgSize-=PAGE_SIZE) {
        if (!(page=PmmFindPage(pAddr, k, 0))) {
            kprintf("[SetupPartitionPgd] Invalid Pgt 0x%x\n",  pAddr);
            return 0;
        }
        ASSERT(page);
        page->type=PPAG_PGT;
        vAddr=(xm_u32_t)VCacheMapPage(pAddr, page);
        memset((xm_u8_t *)vAddr, 0, PAGE_SIZE);
        VCacheUnlockPage(page);
    }
    vPgd=VCacheMapPage(pgd, PmmFindPage(pgd, k, 0));
    memset((xm_u8_t *)vPgd, 0, PAGE_SIZE);
    memcpy((xm_u8_t *)&vPgd[XM_PGD_START], (xm_u8_t *)&xmPgd[XM_PGD_START], sizeof(xm_u32_t)*(XM_PGD_END-XM_PGD_START));

    // Building the partition's virtual memory map
    cPgt=0;
    for (e=0; e<k->ctrl.g->cfg->noPhysicalMemoryAreas; e++) {
        if (!(xmcPhysMemAreaTab[e+k->ctrl.g->cfg->physicalMemoryAreasOffset].flags&XM_MEM_AREA_MAPPED))
            continue;
        pAddr=xmcPhysMemAreaTab[e+k->ctrl.g->cfg->physicalMemoryAreasOffset].startAddr;
        for (; pAddr<xmcPhysMemAreaTab[e+k->ctrl.g->cfg->physicalMemoryAreasOffset].startAddr+xmcPhysMemAreaTab[e+k->ctrl.g->cfg->physicalMemoryAreasOffset].size;) {
            if(pAddr>=CONFIG_XM_OFFSET) {
                kprintf("Partition (%d): Virtual address (0x%x) cannot be mapped\n", k->ctrl.g->cfg->id, pAddr);
                return 0;
            }

            if (!(page=PmmFindPage(pgd, k, &areaFlags))) {
                kprintf("Partition (%d): Page (0x%x) does not belong to the partition\n", k->ctrl.g->cfg->id, pAddr);
                return 0;
            }

            vPgd=VCacheMapPage(pgd, page);

            if (!(vPgd[VA2Pgd(pAddr)]&_PG_PRESENT)) {
                if (cPgt>=noPgts) {
                    kprintf("Not enough PGTs to map all the memory areas of the partition %d\n", k->ctrl.g->cfg->id);
                    return 0;
                }
                pPgt=(pgts+(cPgt*PAGE_SIZE))&PAGE_MASK;
                vPgd[VA2Pgd(pAddr)]=pPgt|_PG_PRESENT|_PG_RW;
                if (!(page=PmmFindPage(pPgt, k, 0)))
                    return 0;
                vPgt=VCacheMapPage(pPgt, page);
                PPAG_INC_COUNT(page);
                cPgt++;
            } else {
                pPgt=vPgd[VA2Pgd(pAddr)]&PAGE_MASK;
                if (!(page=PmmFindPage(pPgt, k, 0)))
                    return 0;
                vPgt=VCacheMapPage(pPgt, page);
            }
            vPgt[VA2Pgt(pAddr)]=pAddr|_PG_PRESENT;
            if (!(page=PmmFindPage(pAddr, k, 0)))
                if (!PmmFindAddr(pAddr, k, 0))
                    return 0;

            if (page) {

                // ensure that only pgd or pgts pages have type != PPAG_STD
                if ( pgd <= pAddr && pAddr <= (pgd+xmgHdr->pagTabSize+PAGE_SIZE) ) {
                    page->counter = 0;
                } else {
                    page->type = PPAG_STD;
                    page->counter = 0;
                }

                PPAG_INC_COUNT(page);
                if ((page->type==PPAG_STD)&&(areaFlags&XM_MEM_AREA_WRITE))
                    vPgt[VA2Pgt(pAddr)]|=_PG_RW;
            } else {
                if (areaFlags&XM_MEM_AREA_WRITE)
                    vPgt[VA2Pgt(pAddr)]|=_PG_RW;
            }
            pAddr+=PAGE_SIZE;
        }
    }
    return pgd;
}

void SetupVmMap(void) {
    xm_u32_t fFlags=GetCpuIdEdx(1);
    xm_s32_t e;
    if (CpuHasPse(fFlags)) {
        // XM is mapped by using a 4MB page, improving the performance of
        // the TLB
        xmPgd[XM_PGD_START]=(_PG_PRESENT|_PG_PSE|_PG_RW|_PG_GLOBAL);
        if (xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size > PSE_PAGE_SIZE) {
            xmPgd[(PSE_PAGE_SIZE+XM_OFFSET)>>PGDIR_SHIFT]=(_PG_PRESENT|_PG_PSE|_PG_RW|_PG_GLOBAL);
        }
    } else {
        if (xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size > (PAGE_SIZE*PDT_ENTRIES)) {
            xmPgd[(PAGE_SIZE*PDT_ENTRIES)>>PGDIR_SHIFT]=_PG_PRESENT|_PG_RW|XMVIRT2PHYS(xmPgt);
            for (e=0; e<PDT_ENTRIES; e++)
                xmPgt[e]=_PG_PRESENT|_PG_RW|_PG_GLOBAL;
        }
    }

    for (e=0; e<XM_PGD_START; e++)
        xmPgd[e]=0;
    FlushTlbGlobal();
}

xm_s32_t VmVAddrIsFree(xm_u32_t vAddr) {
    ASSERT(!(vAddr&(PAGE_SIZE-1)));
    ASSERT(vAddr>=XM_OFFSET);
    return ((!(xmPgt[(vAddr-XM_OFFSET)>>PAGE_SHIFT]&_PG_PRESENT))?1:0);
}

void VmMapPage(xm_u32_t pAddr, xm_u32_t vAddr, xm_u32_t flags) {
    ASSERT(!(pAddr&(PAGE_SIZE-1)));
    ASSERT(!(vAddr&(PAGE_SIZE-1)));
    ASSERT(vAddr>=XM_OFFSET);
    //ASSERT(VmVAddrIsFree(vAddr));
    ASSERT((xmPgd[VA2Pgd(vAddr)]&_PG_PRESENT)==_PG_PRESENT);
    ASSERT(!(xmPgd[VA2Pgd(vAddr)]&_PG_PSE));
    //ASSERT(!(xmPgt[(vAddr-XM_OFFSET)>>PAGE_SHIFT]&_PG_PRESENT));
    xmPgt[(vAddr-XM_OFFSET)>>PAGE_SHIFT]=pAddr|flags;
    FlushTlbEntry(vAddr);
}

void VmUnmapPage(xm_u32_t vAddr) {
    ASSERT(!(vAddr&(PAGE_SIZE-1)));
    ASSERT(vAddr>=XM_OFFSET);
    ASSERT(!VmVAddrIsFree(vAddr));
    ASSERT((xmPgd[VA2Pgd(vAddr)]&_PG_PRESENT)==_PG_PRESENT);
    ASSERT(!(xmPgd[VA2Pgd(vAddr)]&_PG_PSE));
    xmPgt[(vAddr-XM_OFFSET)>>PAGE_SHIFT]=0;
    FlushTlbEntry(vAddr);
}

