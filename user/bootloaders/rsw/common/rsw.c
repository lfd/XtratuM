/*
 * $FILE: rsw.c
 *
 * A boot rsw
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <xm_inc/arch/arch_types.h>
#include <xm_inc/xmef.h>
#include <xm_inc/xmconf.h>
#include <stdc.h>

extern struct xmefPackageHeader xmefHeader;

void HaltSystem(void) {
    extern void _halt_system(void);

    xprintf("[RSW] System Halted.\n");
    _halt_system();
}

void DoTrap(xm_u32_t noTrap) {
    xprintf("[RSW] Unexpected trap: %d\n", noTrap);
    HaltSystem();
}

static struct xmc *SeekXMC(xm_u8_t *xmefCompMap, struct xmefComponent *xmefCompTab, struct xmefFile *xmefFileTab) {
    struct xmc *xmc;
    xm_s32_t e, i;
    
    // Firstly we look for the XM's configuration File
    for (e=0; e<xmefHeader.noComponents; e++) {
        // Hypervisor Found
        if (xmefCompTab[e].flags&CONFIG_COMP_HYPERVISOR_FLAG) {
            for (i=1; i<xmefCompTab[e].noFiles; i++) {
                xmc=(struct xmc *)(&xmefCompMap[xmefFileTab[xmefCompTab[e].fileTabEntry+i].offset]);
                if (xmc->signature==XMC_SIGNATURE)
		    return xmc;
            }
        }
    }

    xprintf("[RSW] XMC file not found\n");
    HaltSystem();
    return 0;
}

#ifdef CONFIG_XEF_SUPPORT
static struct xmImageHdr *FindImgHdr(struct xefHdr *xefHdr) {
    struct xmImageHdr *img=0;
    struct xefSection *sec=(struct xefSection *)((xmAddress_t)xefHdr+xefHdr->secOffset);
    xm_s32_t e;
    for (e=0; e<xefHdr->nSec; e++) {
	img=(struct xmImageHdr *)((xmAddress_t)xefHdr+sec[e].offset);
	if ((img->signature==XMEF_XM_MAGIC)||(img->signature==XMEF_PARTITION_MAGIC))
	    break;
    }
    return img;
}
#endif

static struct xmcPartition *LookForCfg(xm_u32_t id, struct xmc *xmc, struct xmcPartition *partitionTab) {
    xm_s32_t e;

    for (e=0; e<xmc->noPartitions; e++) {
	if (partitionTab[e].imageId==id)
	    return &partitionTab[e];
    }
    return 0;
}

static int CheckFileInsideArea(xmAddress_t start, xmAddress_t size, struct xmcMemoryArea *physMem, xm_s32_t noPhysAreas) {
    xm_s32_t e;
    for (e=0; (e<noPhysAreas); e++)
	if ((start>=physMem[e].startAddr)&&((start+size)<=(physMem[e].startAddr+physMem[e].size))) break;
    
    if (e>=noPhysAreas) {
	xprintf("[RSW] XM/partition [0x%x - 0x%x] is outside its physical memory areas\n", start, start+size);
	HaltSystem();
	return -1;
    }
    return 0;
}

static int CheckFileDontOverlapRSW(xmAddress_t start, xmAddress_t size) {
    extern char _srorsw[], _erorsw[], _srsw[], _ersw[];
    xmAddress_t a0, a1, b0, b1;
    a0=start;
    a1=a0+size;
    b0=(xmAddress_t)_srorsw;
    b1=b0+(xmAddress_t)(_erorsw-_srorsw);

    if (!((a0>=b1)||(b0>=a1))) {
	xprintf("[RSW] XM/partition [0x%x - 0x%x] overlapping RSW memory area [0x%x - 0x%x]\n", a0, a1, b0, b1);
	HaltSystem();
	return -1;
    }
    
    b0=(xmAddress_t)_srsw;
    b1=b0+(xmAddress_t)(_ersw-_srsw);
    if (!((a0>=b1)||(b0>=a1))) {
	xprintf("[RSW] XM/partition [0x%x - 0x%x] overlapping RSW memory area [0x%x - 0x%x]\n", a0, a1, b0, b1);
	HaltSystem();
	return -1;
    }
    return 0;
}

#ifdef CONFIG_XEF_SUPPORT
static xm_s32_t LoadXef(struct xefHdr *xefHdr, xmAddress_t loadAddr, struct xmcMemoryArea *physMem, xm_s32_t noPhysAreas) {
    struct xefSection *xefSec;
    struct xefRel *xefRel;
    xmAddress_t *r_ptr, val;
    xm_s32_t e;
    
    xprintf("[RSW] Loading XEF file at 0x%x\n", loadAddr);
    // xef file format found
    xefSec=(struct xefSection *)((xmAddress_t)xefHdr+xefHdr->secOffset);
    for (e=0; e<xefHdr->nSec; e++) {
	CheckFileInsideArea(loadAddr+xefSec[e].pAddr-xefSec[0].pAddr, xefSec[e].fileSz, physMem, noPhysAreas);
	CheckFileDontOverlapRSW(loadAddr+xefSec[e].pAddr-xefSec[0].pAddr, xefSec[e].fileSz);
	memcpy((xm_u8_t *)(loadAddr+xefSec[e].pAddr-xefSec[0].pAddr), (xm_u8_t *)((xmAddress_t)xefHdr+xefSec[e].offset), xefSec[e].fileSz);
    }

    if (xefHdr->nRel) {
	xprintf("[RSW] Relocating to 0x%x\n", loadAddr);
	xefRel=(struct xefRel *)((xmAddress_t)xefHdr+xefHdr->relOffset);    
	for (e=0; e<xefHdr->nRel; e++) {
	    if (xefRel[e].info!=ABS_REL) continue;
	    r_ptr=(xmAddress_t *)(loadAddr+xefRel[e].offset-xefSec[0].pAddr);
	    val=*r_ptr;
	    val=(xmAddress_t)(loadAddr+val-xefSec[0].pAddr);
	    *r_ptr=val;
	}
    }
    return 0;
}
#endif

static inline void LoadFile(xm_u8_t *dst, xm_u8_t *src, xm_u32_t size, struct xmcMemoryArea *physMem, xm_s32_t noPhysAreas) {
    CheckFileInsideArea((xmAddress_t)dst, size, physMem, noPhysAreas);
    CheckFileDontOverlapRSW((xmAddress_t)dst, size);
    memcpy(dst, src, size);
}

void RSwMain(void) {
    xm_u32_t baseAddr=(xm_u32_t)&xmefHeader;
    struct xmefComponent *xmefCompTab;
    struct xmImageHdr *xmImageHdr, *hpvHdr=(void*)-1;
    struct xmefFile *xmefFileTab;
    xm_u8_t *xmefCompMap;
    //xm_s8_t *xmefStrTab;
    struct xmcMemoryArea *physMemCfg;
    xm_s32_t e, j, xmFound=0, noPhysMemAreas, imgId;
    xmAddress_t loadAddr=0;
#ifdef CONFIG_XEF_SUPPORT
    struct xefHdr *xefHdr=0;
#endif
    struct xmc *xmc;
    struct xmcPartition *partitionTab;
    struct xmcMemoryArea *physMemAreaTab;

#ifdef CONFIG_OUTPUT_ENABLED
    InitOutput();
#endif
    xprintf("[RSW] Start Resident Software\n");
    // Check signature
    if (xmefHeader.signature!=XM_PACKAGE_SIGNATURE) {
	xprintf("[RSW] Bad container signature (0x%x)\n", xmefHeader.signature);
	HaltSystem();
    }
    
    if ((XM_GET_VERSION(xmefHeader.version)!=XMPACK_VERSION)&&(XM_GET_SUBVERSION(xmefHeader.version)!=XMPACK_SUBVERSION)) {
	xprintf("[RSW] Incompatible container version (%d.%d.%d) current version: %d.%d.%d\n", XM_GET_VERSION(xmefHeader.version), XM_GET_SUBVERSION(xmefHeader.version), XM_GET_REVISION(xmefHeader.version), XMPACK_VERSION, XMPACK_SUBVERSION, XMPACK_REVISION);
	HaltSystem();
    }
    
    xmefCompMap=(xm_u8_t *)(baseAddr+xmefHeader.fileDataOffset);
    xmefCompTab=(struct xmefComponent *)(baseAddr+xmefHeader.componentOffset);
    xmefFileTab=(struct xmefFile *)(baseAddr+xmefHeader.fileTabOffset);
    //xmefStrTab=(xm_s8_t *)(baseAddr+xmefHeader.strTabOffset);
    xmc=SeekXMC(xmefCompMap, xmefCompTab, xmefFileTab);
    partitionTab=(struct xmcPartition *)(xmc->partitionTabOffset+(xmAddress_t)xmc);
    physMemAreaTab=(struct xmcMemoryArea *)(xmc->physicalMemoryAreasOffset+(xmAddress_t)xmc);
    // Looking for the hypervisor and the partitions
    for (e=0;e<xmefHeader.noComponents; e++) {
	if (!xmefCompTab[e].noFiles) {
	    xprintf("[RSW] Component (%d) without files???\n", e);
	    HaltSystem();
	}
	if (!(xmefCompTab[e].flags&CONFIG_COMP_HYPERVISOR_FLAG)&&!(xmefCompTab[e].flags&CONFIG_COMP_LOAD_FLAG)) continue;
	
	xmImageHdr=(struct xmImageHdr *)(&xmefCompMap[xmefFileTab[xmefCompTab[e].fileTabEntry].offset]);
	imgId=xmImageHdr->imageId;
#ifdef CONFIG_XEF_SUPPORT
	if (((struct xefHdr *)xmImageHdr)->signature==XEFSIGNATURE) {
	    xefHdr=(struct xefHdr *)xmImageHdr;
	    imgId=((struct xefHdr *)xmImageHdr)->imageId;
	    if (!(xmImageHdr=FindImgHdr(xefHdr))) {
		xprintf("[RSW] (%d) No image hdr inside XEF file\n", e);
		HaltSystem();
	    }
	}
#endif
	// Look for the configuration of this image
	if (xmefCompTab[e].flags&CONFIG_COMP_HYPERVISOR_FLAG) {
	    physMemCfg=&physMemAreaTab[xmc->hpv.physicalMemoryAreasOffset];
	    noPhysMemAreas=xmc->hpv.noPhysicalMemoryAreas;
	    loadAddr=xmc->hpv.loadPhysAddr;
	    if (xmFound) {
 		xprintf("[RSW] Hypervisor already found\n");
 		HaltSystem();
 	    }
	    xmFound++;
	} else {
	    struct xmcPartition *part;
	    if (!(part=LookForCfg(imgId, xmc, partitionTab))) {
		xprintf("[RSW] (%d) imageId (0x%x) not found in container\n", e, imgId);
		HaltSystem();
	    }
	    physMemCfg=&physMemAreaTab[part->physicalMemoryAreasOffset];
	    noPhysMemAreas=part->noPhysicalMemoryAreas;
	    loadAddr=part->loadPhysAddr;
	}


#ifdef CONFIG_XEF_SUPPORT
	if (xefHdr) {	    
	    LoadXef(xefHdr, loadAddr, physMemCfg, noPhysMemAreas);
	} else {
#endif	
	    LoadFile((xm_u8_t *)loadAddr, &xmefCompMap[xmefFileTab[xmefCompTab[e].fileTabEntry].offset], xmefFileTab[xmefCompTab[e].fileTabEntry].size, physMemCfg, noPhysMemAreas);
#ifdef CONFIG_XEF_SUPPORT
	}
#endif

	if (xmefCompTab[e].flags&CONFIG_COMP_HYPERVISOR_FLAG)
	    hpvHdr=(struct xmImageHdr *)loadAddr;
	else
	    xmImageHdr=(struct xmImageHdr *)loadAddr;

	if(xmImageHdr->noModules>CONFIG_MAX_NO_FILES)
        xmImageHdr->noModules=CONFIG_MAX_NO_FILES;
	for (j=0; j<xmImageHdr->noModules; j++) {
	    if (j>=(xmefCompTab[e].noFiles-1)) {
		xmImageHdr->moduleTab[j].size=0;
		continue;
	    }
	    LoadFile((xm_u8_t *)xmImageHdr->moduleTab[j].sAddr, &xmefCompMap[xmefFileTab[xmefCompTab[e].fileTabEntry+j+1].offset], xmefFileTab[xmefCompTab[e].fileTabEntry+j+1].size, physMemCfg, noPhysMemAreas);
	    xmImageHdr->moduleTab[j].size=xmefFileTab[xmefCompTab[e].fileTabEntry+j+1].size;
	}
    }

    if (!xmFound) {
	xprintf("[RSW] No hypervisor found\n");
	HaltSystem();
    }
    xprintf("[RSW] Starting XM at 0x%x\n", hpvHdr->entry.ePoint);
    ((void (*)(void))hpvHdr->entry.ePoint)();
}
