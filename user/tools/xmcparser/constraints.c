/*
 * $FILE: xmc_check.c
 *
 * XMC's c file handling
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include "xmcparser.h"
#include <xm_inc/xmef.h>
#include <xm_inc/bitwise.h>

static struct constraint {
    char *name;
    int (*Checker)(struct xmc *xmcTab);
} *constTab=0;

static int constEntries=0;

struct memArea {
    unsigned long start, end, shared;
    int startLine, endLine;
} *memAreaTab=0;

struct memRegion {
    unsigned long start, end;
    int startLine, endLine;
} *memRegionTab=0;

static int noMemAreas=0, noMemRegions=0;

void AddMemoryArea(unsigned long start, unsigned long end, int stLine, int endLine) {
    memAreaTab=realloc(memAreaTab, (noMemAreas+1)*sizeof(struct memArea));
    memAreaTab[noMemAreas]=(struct memArea){
	.start=start,
	.end=end,
	.shared=0,
	.startLine=stLine,
	.endLine=endLine,
    };
    noMemAreas++;
}

void SetLastMemoryAreaShared(void) {
    memAreaTab[noMemAreas-1].shared=1;
}

void AddMemoryRegion(unsigned long start, unsigned long end, int stLine, int endLine) {
    memRegionTab=realloc(memRegionTab, (noMemRegions+1)*sizeof(struct memRegion));
    memRegionTab[noMemRegions]=(struct memRegion){
	.start=start,
	.end=end,
	.startLine=stLine,
	.endLine=endLine,
    };
    noMemRegions++;
}

static int CheckMemoryAreasOverlap(struct xmc *xmcTab) {
    unsigned long a0, a1, b0, b1;
    int e0, e1;
    
    for (e0=0; e0<noMemAreas-1; e0++) {
	a0=memAreaTab[e0].start;
	a1=memAreaTab[e0].end;
	for (e1=e0+1; e1<noMemAreas; e1++) {
	    b0=memAreaTab[e1].start;
	    b1=memAreaTab[e1].end;
	    if (a0>=b1||b0>=a1)
		continue;
	    else {
		if (!memAreaTab[e1].shared&&!memAreaTab[e0].shared) {
		    if (a0<b1)
			PrintNoLine(memAreaTab[e1].endLine);
		    else
			if (b0<a1)
			    PrintNoLine(memAreaTab[e1].startLine);
		    fprintf(stderr, "Memory area [0x%lx - 0x%lx] overlaps [0x%lx - 0x%lx]\n", a0, a1, b0, b1);
		    return -1;
		}
	    }
	    
	}
    }
    return 0;
}

static int CheckMemoryRegionsOverlap(struct xmc *xmcTab) {
    unsigned long a0, a1, b0, b1;
    int e0, e1;
    
    for (e0=0; e0<noMemRegions-1; e0++) {
	a0=memRegionTab[e0].start;
	a1=memRegionTab[e0].end;
	for (e1=e0+1; e1<noMemRegions; e1++) {
	    b0=memRegionTab[e1].start;
	    b1=memRegionTab[e1].end;
	    if (a0>=b1||b0>=a1)
		continue;
	    else {
		if (a0<b1)
		    PrintNoLine(memRegionTab[e1].endLine);
		else
		    if (b0<a1)
			PrintNoLine(memRegionTab[e1].startLine);
		fprintf(stderr, "Memory region [0x%lx - 0x%lx] overlaps [0x%lx - 0x%lx]\n", a0, a1, b0, b1);
		return -1;
	    }
	    
	}
    }
    return 0;
}

static int CheckMemoryAreaInsideRegion(struct xmc *xmcTab) {
    unsigned long a0, a1, b0=0, b1=0;
    int e0, e1, found=0;
    
    for (e0=0; e0<noMemAreas; e0++) {
	a0=memAreaTab[e0].start;
	a1=memAreaTab[e0].end;
	for (e1=0; e1<noMemRegions; e1++) {
	    b0=memRegionTab[e1].start;
	    b1=memRegionTab[e1].end;
	    if ((a0>=b0)&&(a1<=b1)) {
		found=1;
	    }
	}
	if (!found) {
	    if (a0<b0)
		PrintNoLine(memAreaTab[e0].startLine);
	    else
		if (a1>b1)
		    PrintNoLine(memAreaTab[e0].endLine);
	    fprintf(stderr, "There is no memory region covering [0x%lx - 0x%lx] outside all memory regions\n", a0, a1);
	    return -1;
	}
	found=0;
    }

    return 0;
}

static int CheckHpvLoadAddr(struct xmc *xmcTab) {
    int found, e;
    unsigned long load;
    load=xmcTab->hpv.loadPhysAddr;
    for (found=0, e=0; e<xmcTab->hpv.noPhysicalMemoryAreas; e++) {
	if (physMemAreaTab[e+xmcTab->hpv.physicalMemoryAreasOffset].startAddr<=load&&(physMemAreaTab[e+xmcTab->hpv.physicalMemoryAreasOffset].startAddr+physMemAreaTab[e+xmcTab->hpv.physicalMemoryAreasOffset].size)>load)
		    found=1;
    }
    if (!found) {
	PrintLine(&xmcTab->hpv.loadPhysAddr);
	fprintf(stderr, "XM's load physical address (0x%lx) outside partition memory areas\n", load);
	return -1;
    }
    return 0;
}

static int CheckPartHdrAddrInsideMemArea(struct xmc *xmcTab) {
    int found, e0, e1;
    unsigned long hdr, load;
    
    for (e0=0; e0<xmcTab->noPartitions; e0++) {
	if (partitionTab[e0].flags&XM_PART_BOOT) {
	    load=partitionTab[e0].loadPhysAddr;
	    hdr=partitionTab[e0].headerOffset+load;
/*#ifdef CONFIG_MMU
	    for (found=0, e1=0; e1<partitionTab[e0].noVirtualMemoryAreas; e1++) {
		if (partitionTab[e0].virtualMemoryMap[e1].virtualStartAddr<=hdr&&(partitionTab[e0].virtualMemoryMap[e1].virtualStartAddr+partitionTab[e0].virtualMemoryMap[e1].size)>hdr)
		    found=1;
	    }
	    #else*/
	    for (found=0, e1=0; e1<partitionTab[e0].noPhysicalMemoryAreas; e1++) {
		if (physMemAreaTab[e1+partitionTab[e0].physicalMemoryAreasOffset].startAddr<=load&&(physMemAreaTab[e1+partitionTab[e0].physicalMemoryAreasOffset].startAddr+physMemAreaTab[e1+partitionTab[e0].physicalMemoryAreasOffset].size)>load)
		    found=1;
	    }
//#endif
	    if (!found) {
		PrintLine(&partitionTab[e0].loadPhysAddr);
		fprintf(stderr, "Partition's load physical address (0x%lx) outside partition memory areas\n", load);
		return -1;
	    }

	    for (found=0, e1=0; e1<partitionTab[e0].noPhysicalMemoryAreas; e1++) {
		if (physMemAreaTab[e1+partitionTab[e0].physicalMemoryAreasOffset].startAddr<=hdr&&(physMemAreaTab[e1+partitionTab[e0].physicalMemoryAreasOffset].startAddr+physMemAreaTab[e1+partitionTab[e0].physicalMemoryAreasOffset].size)>hdr)
		    found=1;
	    }
//#endif
	    if (!found) {
		PrintLine(&partitionTab[e0].headerOffset);
		fprintf(stderr, "Partition's header address (0x%lx) outside partition memory areas\n", hdr);
		return -1;
	    }
	}
    }
    
    return 0;
}

void AddConstraint(char *name, int (*Checker)(struct xmc *xmcTab)) {
    constTab=realloc(constTab, (constEntries+1)*sizeof(struct constraint));
    constTab[constEntries].name=malloc(strlen(name)+1);
    strcpy(constTab[constEntries].name, name);
    constTab[constEntries].Checker=Checker;
    constEntries++;
}

void CheckConstraints(struct xmc *xmcTab) {
    int e;
    for (e=0; e<constEntries; e++) {
	if (constTab[e].Checker)
	    if (constTab[e].Checker(xmcTab)<0) {
		ShowErrorMsgAndExit("xmc file breaks constraint %d: \"%s\"", e, constTab[e].name);
	    }
    }
}

static int CheckAllocatedCPU(struct xmc *xmcTab) {
    int e, i, cpu;

    for (e=0; e<xmcTab->noPartitions; e++) {
        cpu=partitionTab[e].flags&XM_PART_CPU_MASK;
        for (i=0; i<xmcTab->hpv.noCpus; i++)
            if (cpu==xmcTab->hpv.cpuTab[i].id)
                break;
	
        if (i>=xmcTab->hpv.noCpus) {
	    PrintLine(&partitionTab[e].flags);
            fprintf(stderr, "(Partition  %d) Cpu Id \"%d\" not found\n", partitionTab[e].id, cpu);
	    return -1;
	}
    }
    return 0;
}

static int CheckPortType(struct xmc *xmcTab) {
    int e, i;

    for (e=0; e<xmcTab->noPartitions; e++) {
	for (i=0; i<partitionTab[e].noPorts; i++) {
	    if (commPorts[i+partitionTab[e].commPortsOffset].channelId!=XM_NULL_CHANNEL) {
		if (commPorts[i+partitionTab[e].commPortsOffset].type!=commChannelTab[commPorts[i+partitionTab[e].commPortsOffset].channelId].type) {
		    PrintLine(&commPorts[i+partitionTab[e].commPortsOffset].type);
		    fprintf(stderr, "(Partition %d) Port \"%s\" and channel type mismatch\n",  partitionTab[e].id, &stringTab[commPorts[i+partitionTab[e].commPortsOffset].nameOffset]);
		    return -1;
		}
	    }
	}
    }
    return 0;
}

static int CheckReplicatedPorts(struct xmc *xmcTab) {
    int e, i, j;

    /* Checking that there are not replicated ports in a same  partition */
    for (e=0; e<xmcTab->noPartitions; e++) {
	for (i=0; i<(partitionTab[e].noPorts-1); i++) {
	    for (j=i+1; j<partitionTab[e].noPorts; j++) {
		if (!strcmp(&stringTab[commPorts[i+partitionTab[e].commPortsOffset].nameOffset], &stringTab[commPorts[j+partitionTab[e].commPortsOffset].nameOffset])) {
		    PrintLine(&stringTab[commPorts[j+partitionTab[e].commPortsOffset].nameOffset]);
		    fprintf(stderr, "(Partition %d): Port Name \"%s\" is duplicated\n", partitionTab[e].id, &stringTab[commPorts[j+partitionTab[e].commPortsOffset].nameOffset]);
		    return -1;
		}
	    }
	}
    }
    return 0;
}

static int CheckCyclicPlan(struct xmc *xmcTab) {
    int e, i, j;
    unsigned long t;
    /* Checking the scheduler params */
    for (e=0; e<xmcTab->hpv.noCpus; e++) {
	if (xmcTab->hpv.cpuTab[e].schedPolicy==XM_SCHED_CYCLIC) {
	    //for (i=0; i<xmcTab->noSchedCyclicPlans; i++) {
	    for (i=0; i<xmcTab->hpv.cpuTab[e].schedParams.cyclic.noSchedCyclicPlans; i++) {
		for (t=0, j=0; j<schedCyclicPlanTab[i].noSlots; j++) {
		    if (t>schedCyclicSlotTab[j+schedCyclicPlanTab[i].slotsOffset].sExec) {
			PrintLine(&schedCyclicSlotTab[j+schedCyclicPlanTab[i].slotsOffset].sExec);
			fprintf(stderr, "Processor \"%d\" plan \"%d\" slot \"%d\": current slot (%lu) overlaps the previous one (%u)\n", e, i, j, t, schedCyclicSlotTab[j+schedCyclicPlanTab[i].slotsOffset].sExec);
			return -1;
		    }
		    t=schedCyclicSlotTab[j+schedCyclicPlanTab[i].slotsOffset].eExec;
		}
		if (t>schedCyclicPlanTab[i].majorFrame) {
		    PrintLine(&schedCyclicPlanTab[i].majorFrame);
		    fprintf(stderr, "Processor \"%d\" plan \"%d\" slot \"%d\": last slot (%lu) overlaps the major frame (%d)\n", e, i, j-1, t, schedCyclicPlanTab[i].majorFrame);
		    return -1;
		}
	    }
	}
    }
    
    return 0;
}

static int CheckCyclicPlanPartitionId(struct xmc *xmcTab) {
    int e0, e1, e2;
    for (e0=0; e0<xmcTab->hpv.noCpus; e0++) {
	if (xmcTab->hpv.cpuTab[e0].schedPolicy==XM_SCHED_CYCLIC) {
	    for (e1=0; e1<xmcTab->hpv.cpuTab[e0].schedParams.cyclic.noSchedCyclicPlans; e1++) {
		for (e2=0; e2<schedCyclicPlanTab[e1].noSlots; e2++) {
		    if ((schedCyclicSlotTab[e2+schedCyclicPlanTab[e1].slotsOffset].partitionId<0)||(schedCyclicSlotTab[e2+schedCyclicPlanTab[e1].slotsOffset].partitionId>=xmcTab->noPartitions)) {
			PrintLine(&schedCyclicSlotTab[e2+schedCyclicPlanTab[e1].slotsOffset].partitionId);
			fprintf(stderr, "Partition \"%d\" does not exist\n", schedCyclicSlotTab[e2+schedCyclicPlanTab[e1].slotsOffset].partitionId);
			return -1;
		    }
		}
	    }
	}
    }
    return 0;
}

static int CheckPartitionId(struct xmc *xmcTab) {
    int e0, e1;
  for (e0=0; e0<xmcTab->noPartitions; e0++)
      if ((partitionTab[e0].id<0)||(partitionTab[e0].id>=xmcTab->noPartitions)) {
	  PrintLine(&partitionTab[e0].id);
	  fprintf(stderr, "Partition's ID \"%d\" invalid\n", partitionTab[e0].id);
	  return -1;
      }

    for (e0=0; e0<xmcTab->noPartitions-1; e0++) {
	for (e1=e0+1; e1<xmcTab->noPartitions; e1++) {
	    if (partitionTab[e0].id==partitionTab[e1].id) {
		PrintLine(&partitionTab[e1].id);
		fprintf(stderr, "Partition's ID \"%d\" duplicated\n", partitionTab[e0].id);
		return -1;
	    }
	    if (!strcmp(&stringTab[partitionTab[e0].nameOffset], &stringTab[partitionTab[e1].nameOffset])) {
		PrintLine(&stringTab[partitionTab[e0].nameOffset]);
		fprintf(stderr, "Partition's name \"%s\" duplicated\n", &stringTab[partitionTab[e0].nameOffset]);
		return -1;
	    }
	}
    }

    return 0;
}

#include <xm_inc/arch/ginfo.h>

static int CheckHwIrqs(struct xmc *xmcTab) {
    int e;
    for (e=0; e<noRsvHwIrqs; e++) {
	if (xmcTab->hpv.hwIrqTab[rsvHwIrqs[e]].owner!=XM_IRQ_NO_OWNER) {
	    PrintLine(&xmcTab->hpv.hwIrqTab[rsvHwIrqs[e]].owner);
	    fprintf(stderr, "Hw irq %d already reserved by XM\n", rsvHwIrqs[e]);
	    return -1;
	}
    }
    return 0;
}

static int CheckIoPorts(struct xmc *xmcTab) {
    unsigned int sysIoPorts[2048]= {[0 ... 2047] = ~0};
    unsigned long a0, a1, b0, b1;
    int e0, e1, e2;
    void *addr;
    
#if defined(CONFIG_IA32)
    for (e0=0; e0<noRsvIoPorts; e0++)
	BitmapClearBits(sysIoPorts, rsvIoPorts[e0].base, rsvIoPorts[e0].offset);

    for (e0=0; e0<xmcTab->noPartitions; e0++) {
	if (partitionTab[e0].noIoPorts<=0)
	    continue;
	for (e1=0; e1<2048; e1++) {
	    a0=~ioPortTab[partitionTab[e0].ioPortsOffset].map[e1]&~sysIoPorts[e1];
	    if (a0) {
		PrintLine(addr);
		fprintf(stderr, "(partitions \"%s\" - XM): IoPort 0x%x ovelapped\n",  &stringTab[partitionTab[e0].nameOffset], _Ffs(a0)+(e1<<5));
		return -1;
	    }
	}
    }
#elif defined(CONFIG_SPARCV8)
    for (e0=0; e0<noRsvIoPorts; e0++) {
	a0=rsvIoPorts[e0].base;
	a1=a0+rsvIoPorts[e0].offset*sizeof(xm_u32_t);
	for (e1=0; e1<xmcTab->noPartitions; e1++) {
	    for (e2=0; e2<partitionTab[e1].noIoPorts; e2++) {
		if (ioPortTab[e2+partitionTab[e1].ioPortsOffset].type==XM_IOPORT_RANGE) {
		    b0=ioPortTab[e2+partitionTab[e1].ioPortsOffset].ioPortRange.base;
		    addr=&ioPortTab[e2+partitionTab[e1].ioPortsOffset].ioPortRange.base;
		    b1=b0+ioPortTab[e2+partitionTab[e1].ioPortsOffset].ioPortRange.noPorts*sizeof(xm_u32_t);
		} else {
		    b0=ioPortTab[e2+partitionTab[e1].ioPortsOffset].restrictedIoPort.address;
		    addr=&ioPortTab[e2+partitionTab[e1].ioPortsOffset].restrictedIoPort.address;
		    b1=b0+sizeof(xm_u32_t);
		}
		
		if (!((a0>=b1)||(a1<=b0))) {
		    PrintLine(addr);		   
		    fprintf(stderr, "IoPorts [0x%lx-0x%lx] (Partition \"%s\") overlaps XM's IO ports [0x%lx-0x%lx]\n", b0, b1, &stringTab[partitionTab[e1].nameOffset], a0, a1);
		    return -1;
		}
	    }
	}
    }
#endif

    return 0;
}

static int CheckPartIoPorts(struct xmc *xmcTab) {
    unsigned long a0, a1, b0, b1;
    int e0, e1, e2, e3;
    void *addr;

    for (e0=0; e0<(xmcTab->noPartitions-1); e0++) {
#if defined(CONFIG_IA32)
	if (partitionTab[e0].noIoPorts<=0)
	    continue;
	for (e1=e0+1; e1<xmcTab->noPartitions; e1++) {
	    if (partitionTab[e1].noIoPorts<=0)
		continue;
	    for (e2=0; e2<2048; e2++) {
		a0=~ioPortTab[partitionTab[e0].ioPortsOffset].map[e2]&~ioPortTab[partitionTab[e1].ioPortsOffset].map[e2];
		if (a0) {
		    PrintLine(addr);
		    fprintf(stderr, "(partitions \"%s\" - \"%s\"): IoPort 0x%x ovelapped\n",  &stringTab[partitionTab[e0].nameOffset], &stringTab[partitionTab[e1].nameOffset], _Ffs(a0)+(e2<<5));
		    return -1;
		}
	    }
	}
#elif defined(CONFIG_SPARCV8)
	for (e1=0; e1<partitionTab[e0].noIoPorts; e1++) {
	    if (ioPortTab[e1+partitionTab[e0].ioPortsOffset].type==XM_IOPORT_RANGE) {
		a0=ioPortTab[e1+partitionTab[e0].ioPortsOffset].ioPortRange.base;
		a1=a0+ioPortTab[e1+partitionTab[e0].ioPortsOffset].ioPortRange.noPorts*sizeof(xm_u32_t);
	    } else {
		a0=ioPortTab[e1+partitionTab[e0].ioPortsOffset].restrictedIoPort.address;
		a1=a0+sizeof(xm_u32_t);
	    }
	    for (e2=(e0+1); e2<xmcTab->noPartitions; e2++) {
		for (e3=0; e3<partitionTab[e2].noIoPorts; e3++) {
		    if (ioPortTab[e3+partitionTab[e2].ioPortsOffset].type==XM_IOPORT_RANGE) {
			b0=ioPortTab[e3+partitionTab[e2].ioPortsOffset].ioPortRange.base;
			addr=&ioPortTab[e3+partitionTab[e2].ioPortsOffset].ioPortRange.base;
			b1=b0+ioPortTab[e3+partitionTab[e2].ioPortsOffset].ioPortRange.noPorts*sizeof(xm_u32_t);
		    } else {
			b0=ioPortTab[e3+partitionTab[e2].ioPortsOffset].restrictedIoPort.address;
			addr=&ioPortTab[e3+partitionTab[e2].ioPortsOffset].restrictedIoPort.address;
			b1=b0+sizeof(xm_u32_t);
		    }
		    if (!((a0>=b1)||(a1<=b0))) {
			if ((ioPortTab[e1+partitionTab[e0].ioPortsOffset].type==XM_RESTRICTED_IOPORT)&&(ioPortTab[e3+partitionTab[e2].ioPortsOffset].type==XM_RESTRICTED_IOPORT)) {
			    if (!(ioPortTab[e1+partitionTab[e0].ioPortsOffset].restrictedIoPort.mask&ioPortTab[e3+partitionTab[e2].ioPortsOffset].restrictedIoPort.mask)) break;
			}
			PrintLine(addr);
			fprintf(stderr, "IoPorts [0x%lx-0x%lx] (Partition \"%s\") overlaps IO ports [0x%lx-0x%lx] (Partition \"%s\")\n", b0, b1, &stringTab[partitionTab[e0].nameOffset], a0, a1, &stringTab[partitionTab[e2].nameOffset]);
			return -1;
		    }
		}
	    }
	}
#endif
    }
    
    return 0;
}

static int CheckPartSelfIoPorts(struct xmc *xmcTab) {
    unsigned long a0, a1, b0, b1;
    int e0, e1, e2;
    void *addr;
    for (e0=0; e0<xmcTab->noPartitions; e0++) {
#if defined(CONFIG_IA32)
#elif defined(CONFIG_SPARCV8)
	for (e1=0; e1<(partitionTab[e0].noIoPorts-1); e1++) {
	    if (ioPortTab[e1+partitionTab[e0].ioPortsOffset].type==XM_IOPORT_RANGE) {
		a0=ioPortTab[e1+partitionTab[e0].ioPortsOffset].ioPortRange.base;
		a1=a0+ioPortTab[e1+partitionTab[e0].ioPortsOffset].ioPortRange.noPorts*sizeof(xm_u32_t);
	    } else {
		a0=ioPortTab[e1+partitionTab[e0].ioPortsOffset].restrictedIoPort.address;
		a1=a0+sizeof(xm_u32_t);
	    }
	    for (e2=(e1+1); e2<partitionTab[e0].noIoPorts; e2++) {
		if (ioPortTab[e2+partitionTab[e0].ioPortsOffset].type==XM_IOPORT_RANGE) {
		    b0=ioPortTab[e2+partitionTab[e0].ioPortsOffset].ioPortRange.base;
		    b1=b0+ioPortTab[e2+partitionTab[e0].ioPortsOffset].ioPortRange.noPorts*sizeof(xm_u32_t);
		    addr=&ioPortTab[e2+partitionTab[e0].ioPortsOffset].ioPortRange.base;
		} else {
		    b0=ioPortTab[e2+partitionTab[e0].ioPortsOffset].restrictedIoPort.address;
		    b1=b0+sizeof(xm_u32_t);
		    addr=&ioPortTab[e2+partitionTab[e0].ioPortsOffset].restrictedIoPort.address;
		}
		if (!((a0>=b1)||(a1<=b0))) {
		    PrintLine(addr);
		    fprintf(stderr, "IoPorts (Partition \"%s\") [0x%lx-0x%lx] overlaps IO ports [0x%lx-0x%lx]\n", &stringTab[partitionTab[e0].nameOffset], b0, b1, a0, a1);
		    return -1;
		}
	    }
	}
#endif
    }
    
    return 0;
}

static int CheckPartIoPortsAlign(struct xmc *xmcTab) {
    unsigned long a0;
    int e0, e1;
    void *addr;

    for (e0=0; e0<xmcTab->noPartitions; e0++) {
#if defined(CONFIG_IA32)
#elif defined(CONFIG_SPARCV8)
	for (e1=0; e1<partitionTab[e0].noIoPorts; e1++) {
	    if (ioPortTab[e1+partitionTab[e0].ioPortsOffset].type==XM_IOPORT_RANGE) {
		a0=ioPortTab[e1+partitionTab[e0].ioPortsOffset].ioPortRange.base;
		addr=&ioPortTab[e1+partitionTab[e0].ioPortsOffset].ioPortRange.base;
	    } else {
		a0=ioPortTab[e1+partitionTab[e0].ioPortsOffset].restrictedIoPort.address;
		addr=&ioPortTab[e1+partitionTab[e0].ioPortsOffset].restrictedIoPort.address;
	    }
	    if (a0&3) {
		PrintLine(addr);
		fprintf(stderr, "IoPorts (Partition \"%s\") 0x%lx base port not aligned\n", &stringTab[partitionTab[e0].nameOffset], a0);
		return -1;
	    }	
	}
#endif
    }
    
    return 0;
}

void InitConstraints(void) {
    AddConstraint("Memory areas overlapping", CheckMemoryAreasOverlap);
    AddConstraint("Memory regions overlapping", CheckMemoryRegionsOverlap);
    AddConstraint("Memory area inside any region", CheckMemoryAreaInsideRegion);
    AddConstraint("Partition's header inside its memory area", CheckPartHdrAddrInsideMemArea);
    AddConstraint("Duplicated Partition's name and id", CheckPartitionId);
    AddConstraint("Allocated Cpus", CheckAllocatedCPU);
    AddConstraint("Replicated port's names and id", CheckReplicatedPorts);
    AddConstraint("Port and channel's type mismatch", CheckPortType);
    AddConstraint("Cyclic scheduling plan", CheckCyclicPlan);
    AddConstraint("Cyclic scheduling plan slot partition id", CheckCyclicPlanPartitionId);
    AddConstraint("HwIrqs allocated to partitions", CheckHwIrqs);
    AddConstraint("Io port alignment", CheckPartIoPortsAlign);
    AddConstraint("Io ports allocated to partitions (1)", CheckIoPorts);
    AddConstraint("Io ports allocated to partitions (2)", CheckPartSelfIoPorts);
    AddConstraint("Io ports allocated to partitions (3)", CheckPartIoPorts);
    AddConstraint("Hpv load address inside its memory area", CheckHpvLoadAddr);
}
