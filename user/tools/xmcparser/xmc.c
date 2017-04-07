/*
 * $FILE: xmc.c
 *
 * XMC's c file handling
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include "xmcparser.h"
#include "devices.h"
#include "hm.h"
#include <xm_inc/xmef.h>

char *xmlFile;
struct xmc xmcTab;
struct xmcPartition *partitionTab=0;
struct xmcMemoryRegion *memRegTab=0;
struct xmcCommChannel *commChannelTab=0;
struct xmcMemoryArea *physMemAreaTab=0;
struct xmcCommPort *commPorts=0;
struct xmcIoPort *ioPortTab=0;
struct xmcSchedCyclicSlot *schedCyclicSlotTab=0;
struct xmcSchedCyclicPlan *schedCyclicPlanTab=0;
char *stringTab=0;

static int lineTabLen=0;
static struct line { 
    int line;
    void *addr;
} *lineTab=0;

void PrintNoLine(int line) {
    if (line>=0)
	fprintf(stderr, "%s:%d: ", xmlFile, line);
    else
	fprintf(stderr, "%s:??: ", xmlFile);
}

void PrintLine(void *addr) {
    int line;
    if ((line=GetLine(addr))>=0)
	fprintf(stderr, "%s:%d: ", xmlFile, line);
    else
	fprintf(stderr, "%s:??: ", xmlFile);
}

void AddLine(void *addr, int line) {
    lineTab=realloc(lineTab, (lineTabLen+1)*sizeof(struct line));
    lineTab[lineTabLen].line=line;
    lineTab[lineTabLen].addr=addr;
    lineTabLen++;
}

int GetLine(void *addr) {
    int e;
    for (e=0; e<lineTabLen; e++)
	if (lineTab[e].addr==addr)
	    return lineTab[e].line;
    return -1;
}

void InitPartitionEntry(struct xmcPartition *part) {
    memset(part, 0, sizeof(struct xmcPartition));
    part->trace.dev.id=XM_DEV_INVALID_ID;
    part->consoleDev.id=XM_DEV_INVALID_ID;
    SetDefPartHmTab(part->hmTab);
}

void InitXMCDefaults(struct xmc *xmcTab) {
    int e;
    
    memset((xm_u8_t *)xmcTab, 0, sizeof(struct xmc));
    for (e=0; e<CONFIG_NO_HWIRQS; e++) {
	xmcTab->hpv.hwIrqTab[e].owner=XM_IRQ_NO_OWNER;
    } 
    SetDefHpvHmTab(xmcTab->hpv.hmTab);
    xmcTab->hpv.consoleDev.id=XM_DEV_INVALID_ID;
    xmcTab->hpv.hmDev.id=XM_DEV_INVALID_ID;
    xmcTab->hpv.trace.dev.id=XM_DEV_INVALID_ID;
 }

static inline int PartitionId2Entry(int id, struct xmc *xmcTab, int line) {
    int e;
    for (e=0; e<xmcTab->noPartitions; e++)
	if (partitionTab[e].id==id)
	    return e;
    PrintNoLine(line);
    ShowErrorMsgAndExit("Partition id \"%d\" not found", id);
    return 0;
}

#if defined(CONFIG_SPARCV8)
static int ComparIoPort(struct xmcIoPort *p0, struct xmcIoPort *p1) {
    int a0, b0;

    if (p0->type==XM_IOPORT_RANGE) {
	a0=p0->ioPortRange.base;
    } else {
	a0=p0->restrictedIoPort.address;
    }
    if (p1->type==XM_IOPORT_RANGE) {
	b0=p1->ioPortRange.base;
    } else {
	b0=p1->restrictedIoPort.address;
    }
    if (a0>b0)
	return 1;
    if (a0<b0)
	return -1;
    return 0;
}
#endif

int ProcessXMC(struct xmc *xmcTab) {
    int e, partition, i, j, found;
    xmAddress_t a0, a1, b0, b1;

    /* Binding the channels with the links */
    for (e=0; e<xmcTab->noCommChannels; e++) {
	for (i=0; i<linkTab[e].noLinks; i++) {
	    partition=PartitionId2Entry(linkTab[e].partitionInfo[i].partitionId, xmcTab, linkTab[e].partitionInfo[i].partIdLine);
	    if (strlen(linkTab[e].partitionInfo[i].partitionName)&&strcmp(&stringTab[partitionTab[partition].nameOffset],linkTab[e].partitionInfo[i].partitionName)) {
		PrintNoLine(linkTab[e].partitionInfo[i].partNameLine);
		ShowErrorMsgAndExit("Partition name \"%s\" does not match with channel's partition name \"%s\"", &stringTab[partitionTab[partition].nameOffset], linkTab[e].partitionInfo[i].partitionName);
	    }
	    
	    for (found=0, j=0; j<partitionTab[partition].noPorts; j++) {
		if (!strcmp(&stringTab[commPorts[j+partitionTab[partition].commPortsOffset].nameOffset], linkTab[e].partitionInfo[i].portName)) {		    
		    if (commPorts[j+partitionTab[partition].commPortsOffset].channelId!=XM_NULL_CHANNEL) {
			PrintNoLine(linkTab[e].partitionInfo[i].portNameLine);
			ShowErrorMsgAndExit("Port \"%s\" already linked with channel \"%d\"", &stringTab[commPorts[j+partitionTab[partition].commPortsOffset].nameOffset], commPorts[j+partitionTab[partition].commPortsOffset].channelId);
		    }
		    commPorts[j+partitionTab[partition].commPortsOffset].channelId=e;
		    found=1;
		}
	    }
	    if (!found) {
		PrintNoLine(linkTab[e].partitionInfo[i].portNameLine);
		ShowErrorMsgAndExit("Port \"%s\" declared on channel not found", linkTab[e].partitionInfo[i].portName);
	    }
	}
    }

    /* Binding the devices */
    LookUpDevice(GetDevAsocName(xmcTab->hpv.trace.dev.id), &xmcTab->hpv.trace.dev);
    for (e=0; e<xmcTab->noPartitions; e++) {
	LookUpDevice(GetDevAsocName(partitionTab[e].trace.dev.id), &partitionTab[e].trace.dev);
	LookUpDevice(GetDevAsocName(partitionTab[e].consoleDev.id), &partitionTab[e].consoleDev);
    }
    LookUpDevice(GetDevAsocName(xmcTab->hpv.hmDev.id), &xmcTab->hpv.hmDev);  
    LookUpDevice(GetDevAsocName(xmcTab->hpv.consoleDev.id), &xmcTab->hpv.consoleDev);
#if defined(CONFIG_SPARCV8)
    for (e=0; e<xmcTab->noPartitions; e++) {
	qsort(&ioPortTab[partitionTab[e].ioPortsOffset], partitionTab[e].noIoPorts, sizeof(struct xmcIoPort), (int(*)(const void *, const void *))ComparIoPort);
    }
#endif
#if defined(CONFIG_IA32)
    for (e=0; e<xmcTab->noIoPorts; e++)
	ioPortTab[e].map[2047]=0xff000000;
#endif

    for (e=0; e<xmcTab->noPhysicalMemoryAreas; e++) {
	a0=physMemAreaTab[e].startAddr;
	a1=a0+physMemAreaTab[e].size-1;
	for (i=0; i<xmcTab->noRegions; i++) {
	    b0=memRegTab[i].startAddr;
	    b1=b0+memRegTab[i].size-1;
	    if ((a0>=b0)&&(a1<=b1)) {
		physMemAreaTab[e].memoryRegionOffset=i;
		break;
	    }
	}
    }
    return 0;
}
