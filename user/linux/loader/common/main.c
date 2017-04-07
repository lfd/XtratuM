/*
 * $FILE: main.c
 *
 * LinuxLoader
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <config.h>
#include <loader.h>
#include <stdc.h>
#include <xm_inc/xmef.h>

/*
xmAddress_t LowestAddress(void) {
    struct xmcMemoryArea *memAreas;
    xmAddress_t lowest=0;
    xm_s32_t e;

    memAreas=(struct xmcMemoryArea *)((xmAddress_t)&partitionInformationTable+partitionInformationTable.physicalMemoryAreasOffset);
    for (e=0; e<partitionInformationTable.noPhysicalMemoryAreas; e++)
	if (memAreas[e].startAddr>=1024*1024) {
	    lowest=memAreas[e].startAddr;
	    break;
	}
    
    for (; e<partitionInformationTable.noPhysicalMemoryAreas; e++)
	if ((memAreas[e].startAddr>=1024*1024)&&(lowest>memAreas[e].startAddr))
	    lowest=memAreas[e].startAddr;
    
    return lowest;   
}
*/

static xm_s32_t LoadXef(struct xefHdr *xefHdr, xmAddress_t loadAddr, xmAddress_t relocAddr) {
    struct xefSection *xefSec;
    struct xefRel *xefRel;
    xmAddress_t *r_ptr, val;
    xm_s32_t e;
    
    xprintf("[LinuxLoader] Loading XEF file at 0x%x\n", loadAddr);
    // xef file format found
    xefSec=(struct xefSection *)((xmAddress_t)xefHdr+xefHdr->secOffset);
    for (e=0; e<xefHdr->nSec; e++) {
	memcpy((xm_u8_t *)(loadAddr+xefSec[e].pAddr-xefSec[0].pAddr), (xm_u8_t *)((xmAddress_t)xefHdr+xefSec[e].offset), xefSec[e].fileSz);
    }

    if (xefHdr->nRel) {
	xprintf("[LinuxLoader] Relocating to 0x%x\n", relocAddr);
	xefRel=(struct xefRel *)((xmAddress_t)xefHdr+xefHdr->relOffset);
	for (e=0; e<xefHdr->nRel; e++) {
	    if (xefRel[e].info!=ABS_REL) continue;
	    r_ptr=(xmAddress_t *)(loadAddr+xefRel[e].offset-xefSec[0].vAddr);
	    val=*r_ptr;
	    val=(xmAddress_t)(relocAddr+val-xefSec[0].vAddr);
	    *r_ptr=val;
	}
    }
    return relocAddr+xefHdr->entry-xefSec[0].pAddr;
}

void LinuxLoader(void) {
    struct xefHdr *hdr;
    xmAddress_t ePoint;

    xprintf("Linux loader "LINUX_LOADER_VERSION"\n");
    if ((partitionControlTable.magic!=KTHREAD_MAGIC)||(partitionInformationTable.signature!=PARTITION_INFORMATION_TABLE_SIGNATURE)) {
        xprintf("[LinuxLoader] partitionControlTable or partitionInformationTable corrupted\n");
        return;
    }
    hdr=(struct xefHdr *)_slinuxImg;
    SetupVMMap(LINUX_OFFSET);
    if (hdr->signature==XEFSIGNATURE) {	
        ePoint=LoadXef(hdr, (xmAddress_t)_slinuxloader, (xmAddress_t)_slinuxloader+LINUX_OFFSET);
    }  else {
        xprintf("Unsupported image format\n");
        return;
    }
    
    xprintf("[LinuxLoader] Jumping to 0x%x\n", ePoint);
    JumpToLinux(ePoint);
}
