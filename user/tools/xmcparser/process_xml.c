/*
 * $FILE: process_xml.c
 *
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <string.h>
#include "hm.h"
#include "xmcparser.h"
#include "devices.h"
#include "constraints.h"

struct linkTab *linkTab=0;

extern void FeatCpuAttrHandler(xmlNodePtr node, const xmlChar *val);

static void PeriodTempReqAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].temporalRestrictions.period=TimeStr2Time((char *)val);
    AddLine(&partitionTab[CURRENT_PARTITION].temporalRestrictions.period, node->line);
}

static void DurationTempReqAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].temporalRestrictions.duration=TimeStr2Time((char *)val);
    AddLine(&partitionTab[CURRENT_PARTITION].temporalRestrictions.duration, node->line);
}

static void IdPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].id=atoi((char *)val);
    //AddPartition(partitionTab[CURRENT_PARTITION].id);
    AddLine(&partitionTab[CURRENT_PARTITION].id, node->line);
}

static void NamePartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].nameOffset=xmcTab.stringTabLength;
    xmcTab.stringTabLength+=(strlen((char *)val)+1);
    stringTab=realloc(stringTab, xmcTab.stringTabLength);
    strcpy(&stringTab[partitionTab[CURRENT_PARTITION].nameOffset], (char *)val);
    //strncpy(partitionTab[CURRENT_PARTITION].name, (char *)val,CONFIG_ID_STRING_LENGTH);
    //partitionTab[CURRENT_PARTITION].name[CONFIG_ID_STRING_LENGTH-1]=0x0;
    AddLine(&stringTab[partitionTab[CURRENT_PARTITION].nameOffset], node->line);
}

static void ProcessorPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    int cpu=atoi((char *)val);
    partitionTab[CURRENT_PARTITION].flags&=~XM_PART_CPU_MASK;
    partitionTab[CURRENT_PARTITION].flags|=(cpu&XM_PART_CPU_MASK);
    AddLine(&partitionTab[CURRENT_PARTITION].flags, node->line);
}

static void FlagsPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    char *tmp, *tmp1;

    for (tmp=(char *)val, tmp1=strstr(tmp, " "); tmp; tmp=((tmp1)?tmp1+1:0), tmp1=strstr(tmp, " ")) {
        if (tmp1) *tmp1=0;
        if (strcmp(tmp, "sv")==0) {
	    partitionTab[CURRENT_PARTITION].flags|=XM_PART_SUPERVISOR;
        }
	if (strcmp(tmp, "boot")==0) {
	    partitionTab[CURRENT_PARTITION].flags|=XM_PART_BOOT;
        }
#ifdef CONFIG_SPARE_SCHEDULING
	if (strcmp(tmp, "sphost")==0) {
            partitionTab[CURRENT_PARTITION].flags|=XM_PART_SPARE_H;
        }
	if (strcmp(tmp, "spguest")==0) {
            partitionTab[CURRENT_PARTITION].flags|=XM_PART_SPARE_G;
        }
#endif
    }
    AddLine(&partitionTab[CURRENT_PARTITION].flags, node->line);
}

static void FPPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    if (!xmlStrcasecmp(val, (xmlChar *)"yes")||!xmlStrcasecmp(val, (xmlChar *)"true"))
        partitionTab[CURRENT_PARTITION].flags|=XM_PART_FP;
    else
        partitionTab[CURRENT_PARTITION].flags&=~XM_PART_FP;
    AddLine(&partitionTab[CURRENT_PARTITION].flags, node->line);
}

static void LoadPhysAddrPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].loadPhysAddr=strtoul((char *)val, 0, 16);
    AddLine(&partitionTab[CURRENT_PARTITION].loadPhysAddr, node->line);
}

static void BootEntryPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].headerOffset=strtoul((char *)val, 0, 16);
    AddLine(&partitionTab[CURRENT_PARTITION].headerOffset, node->line);
}

static void ImgIdPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].imageId=strtoul((char *)val, 0, 16);
    AddLine(&partitionTab[CURRENT_PARTITION].imageId, node->line);
}

static void ValidPeriodChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    commChannelTab[CURRENT_COMM_CHANNEL].validPeriod=TimeStr2Time((char *)val);
    AddLine(&commChannelTab[CURRENT_COMM_CHANNEL].validPeriod, node->line);
}

static void MaxMsgLenSChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    commChannelTab[CURRENT_COMM_CHANNEL].s.maxLength=SizeStr2Size((char *)val);
    AddLine(&commChannelTab[CURRENT_COMM_CHANNEL].s.maxLength, node->line);
}

static void MaxMsgLenQChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    commChannelTab[CURRENT_COMM_CHANNEL].q.maxLength=SizeStr2Size((char *)val);
    AddLine(&commChannelTab[CURRENT_COMM_CHANNEL].q.maxLength, node->line);
}

static void MaxNoMsgsQChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    commChannelTab[CURRENT_COMM_CHANNEL].q.maxNoMsgs=SizeStr2Size((char *)val);
    AddLine(&commChannelTab[CURRENT_COMM_CHANNEL].q.maxNoMsgs, node->line);
}

static void SamplingChannelHandler(xmlNodePtr node) {
    xmcTab.noCommChannels++;
    commChannelTab=realloc(commChannelTab, sizeof(struct xmcCommChannel)*xmcTab.noCommChannels);
    memset(&commChannelTab[CURRENT_COMM_CHANNEL], 0, sizeof(struct xmcCommChannel));
    linkTab=realloc(linkTab, sizeof(struct linkTab)*xmcTab.noCommChannels);
    memset(&linkTab[CURRENT_COMM_CHANNEL], 0, sizeof(struct linkTab));
    commChannelTab[CURRENT_COMM_CHANNEL].type=XM_SAMPLING_CHANNEL;
    AddLine(&commChannelTab[CURRENT_COMM_CHANNEL].type, node->line);
}

static void QueuingChannelHandler(xmlNodePtr node) {
    xmcTab.noCommChannels++;
    commChannelTab=realloc(commChannelTab, sizeof(struct xmcCommChannel)*xmcTab.noCommChannels);
    memset(&commChannelTab[CURRENT_COMM_CHANNEL], 0, sizeof(struct xmcCommChannel));
    linkTab=realloc(linkTab, sizeof(struct linkTab)*xmcTab.noCommChannels);
    memset(&linkTab[CURRENT_COMM_CHANNEL], 0, sizeof(struct linkTab));
    commChannelTab[CURRENT_COMM_CHANNEL].type=XM_QUEUING_CHANNEL;
    AddLine(&commChannelTab[CURRENT_COMM_CHANNEL].type, node->line);
}

static void PartitionHandler(xmlNodePtr node) {    
    extern void InitPartitionEntry(struct xmcPartition *part);
    xmcTab.noPartitions++;
    partitionTab=realloc(partitionTab, xmcTab.noPartitions*sizeof(struct xmcPartition));
    InitPartitionEntry(&partitionTab[CURRENT_PARTITION]);
}

static void VersionSDAttrHandler(xmlNodePtr node, const xmlChar *val) {
    unsigned int version, subversion, revision;

    sscanf((char *)val, "%u.%u.%u", &version, &subversion, &revision);
    xmcTab.fileVersion=XMC_SET_VERSION(version, subversion, revision);
    AddLine(&xmcTab.fileVersion, node->line);
}

static void NameSDAttrHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.nameOffset=xmcTab.stringTabLength;
    xmcTab.stringTabLength+=(strlen((char *)val)+1);
    stringTab=realloc(stringTab, xmcTab.stringTabLength);
    strcpy(&stringTab[xmcTab.nameOffset], (char *)val);
    AddLine(&stringTab[xmcTab.nameOffset], node->line);
}

static void LoadPhysAddrHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.loadPhysAddr=strtoul((char *)val, 0, 16);
    AddLine(&xmcTab.hpv.loadPhysAddr, node->line);
}

static void PhysMemAreasHpvHandler(xmlNodePtr node) {
    xmcTab.hpv.physicalMemoryAreasOffset=xmcTab.noPhysicalMemoryAreas;
}

static void MemAreaHpvHandler(xmlNodePtr node) {
    xmcTab.hpv.noPhysicalMemoryAreas++;
    xmcTab.noPhysicalMemoryAreas++;
    physMemAreaTab=realloc(physMemAreaTab, sizeof(struct xmcMemoryArea)*xmcTab.noPhysicalMemoryAreas);
    memset(&physMemAreaTab[CURRENT_PHYSMEMAREA], 0, sizeof(struct xmcMemoryArea));
}

static void StartMemAreaHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr=strtoul((char *)val,0, 16);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr, node->line);
}

static void SizeMemAreaHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    physMemAreaTab[CURRENT_PHYSMEMAREA].size=SizeStr2Size((char *)val);
    if ((physMemAreaTab[CURRENT_PHYSMEMAREA].size+1024*1024) & 0x3FFFFF)
        ShowErrorMsgAndExit("XtratuM allocated memory must be equal to [3MB | 7MB | 11MB | ...]");
    AddMemoryArea(physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr, physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr+physMemAreaTab[CURRENT_PHYSMEMAREA].size-1, GetLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr), node->line);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].size, node->line);
}

static void SetMemAreaFlags(xm_u32_t *flags, const xmlChar *val) {
    char *tmp, *tmp1;
    
    for (tmp=(char *)val, tmp1=strstr(tmp, " "); tmp; tmp=((tmp1)?tmp1+1:0), tmp1=strstr(tmp, " ")) {
        if (tmp1) *tmp1=0;

	if (!strcasecmp(tmp, "mapped")) {
	    (*flags)|=XM_MEM_AREA_MAPPED;
	} else if (!strcasecmp(tmp, "shared")) {
	    (*flags)|=XM_MEM_AREA_SHARED;
	    SetLastMemoryAreaShared();
	} else if (!strcasecmp(tmp, "write")) {
	    (*flags)|=XM_MEM_AREA_WRITE;
	} else if (!strcasecmp(tmp, "rom")) {
	    (*flags)|=XM_MEM_AREA_ROM;
	} else if (!strcasecmp(tmp, "flag0")) {
	    (*flags)|=XM_MEM_AREA_FLAG0;
	} else if (!strcasecmp(tmp, "flag1")) {
	    (*flags)|=XM_MEM_AREA_FLAG1;
	} else if (!strcasecmp(tmp, "flag2")) {
	    (*flags)|=XM_MEM_AREA_FLAG2;
	} else if (!strcasecmp(tmp, "flag3")) {
	    (*flags)|=XM_MEM_AREA_FLAG3;
	} 
    }
}

static void FlagsMemAreaHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    SetMemAreaFlags(&physMemAreaTab[CURRENT_PHYSMEMAREA].flags, val);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].flags, node->line);
}

static void PhysMemAreasRswHandler(xmlNodePtr node) {
    xmcTab.rsw.physicalMemoryAreasOffset=xmcTab.noPhysicalMemoryAreas;
}

static void MemAreaRswHandler(xmlNodePtr node) {
    xmcTab.rsw.noPhysicalMemoryAreas++;
    xmcTab.noPhysicalMemoryAreas++;
    physMemAreaTab=realloc(physMemAreaTab, sizeof(struct xmcMemoryArea)*xmcTab.noPhysicalMemoryAreas);
    memset(&physMemAreaTab[CURRENT_PHYSMEMAREA], 0, sizeof(struct xmcMemoryArea));
}

static void EntryPointRswAttrHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.rsw.entryPoint=strtoul((char *)val,0, 16);
    AddLine(&xmcTab.rsw.entryPoint, node->line);
}

static void StartMemAreaRswAttrHandler(xmlNodePtr node, const xmlChar *val) {
    physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr=strtoul((char *)val,0, 16);
    AddLine(& physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr, node->line);
}

static void SizeMemAreaRswAttrHandler(xmlNodePtr node, const xmlChar *val) {
    physMemAreaTab[CURRENT_PHYSMEMAREA].size=SizeStr2Size((char *)val);
    AddMemoryArea(physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr, physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr+physMemAreaTab[CURRENT_PHYSMEMAREA].size-1, GetLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr), node->line);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].size, node->line);
}

static void FlagsMemAreaRswAttrHandler(xmlNodePtr node, const xmlChar *val) {
    
    SetMemAreaFlags(&physMemAreaTab[CURRENT_PHYSMEMAREA].flags, val);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].flags, node->line);

}

static void PhysMemAreasPartHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].physicalMemoryAreasOffset=xmcTab.noPhysicalMemoryAreas;
}

static void MemAreaPartHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].noPhysicalMemoryAreas++;
    xmcTab.noPhysicalMemoryAreas++;
    physMemAreaTab=realloc(physMemAreaTab, sizeof(struct xmcMemoryArea)*xmcTab.noPhysicalMemoryAreas);
    memset(&physMemAreaTab[CURRENT_PHYSMEMAREA], 0, sizeof(struct xmcMemoryArea));
}

static void StartMemAreaPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr=strtoul((char *)val,0, 16);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr, node->line);
}

static void SizeMemAreaPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    physMemAreaTab[CURRENT_PHYSMEMAREA].size=SizeStr2Size((char *)val);
    AddMemoryArea(physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr, physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr+physMemAreaTab[CURRENT_PHYSMEMAREA].size-1, GetLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].startAddr), node->line);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].size, node->line);
}

static void FlagsMemAreaPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    SetMemAreaFlags(&physMemAreaTab[CURRENT_PHYSMEMAREA].flags, val);
    AddLine(&physMemAreaTab[CURRENT_PHYSMEMAREA].flags, node->line);
}

static int hmEvent;
static void NameHmEventAttrHandler(xmlNodePtr node, const xmlChar *val) {
    int e;
    for (e=0; e<XM_HM_MAX_EVENTS; e++)
	if (!xmlStrcasecmp(val, (xmlChar *)hmEvents[e])) {
	    //partitionTab[CURRENT_PARTITION].hmTab[e];
	    hmEvent=e;
	    return;
	}
    PrintNoLine(node->line);
    ShowErrorMsgAndExit("HM event name \"%s\" unknown", val);
}

static void ActionHmEventPartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    int e;
    for (e=0; e<MAX_HM_ACTIONS; e++)
	if (!xmlStrcasecmp(val, (xmlChar *)hmActions[e])) {
	    partitionTab[CURRENT_PARTITION].hmTab[hmEvent].action=e;
	    AddLine(&partitionTab[CURRENT_PARTITION].hmTab[hmEvent], node->line);
	    return;
	}
    PrintNoLine(node->line);
    ShowErrorMsgAndExit("HM action name \"%s\" unknown", val);
}

static void LogHmEventPartAttrHandler(xmlNodePtr node, const xmlChar *val) {    
    if (!xmlStrcasecmp(val, (xmlChar *)"yes")||!xmlStrcasecmp(val, (xmlChar *)"true"))
        partitionTab[CURRENT_PARTITION].hmTab[hmEvent].log=XM_HM_LOG_ENABLED;
    else
        partitionTab[CURRENT_PARTITION].hmTab[hmEvent].log=XM_HM_LOG_DISABLED;   
    AddLine(&partitionTab[CURRENT_PARTITION].hmTab[hmEvent], node->line);
}

static void ActionHmEventHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    int e;
    for (e=0; e<MAX_HM_ACTIONS; e++)
	if (!xmlStrcasecmp(val, (xmlChar *)hmActions[e])) {
	    xmcTab.hpv.hmTab[hmEvent].action=e;
	    AddLine(&xmcTab.hpv.hmTab[hmEvent], node->line);
	    return;
	}
    PrintNoLine(node->line);
    ShowErrorMsgAndExit("HM action name \"%s\" unknown", val);
}

static void LogHmEventHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    if (!xmlStrcasecmp(val, (xmlChar *)"yes")||!xmlStrcasecmp(val, (xmlChar *)"true"))
        xmcTab.hpv.hmTab[hmEvent].log=XM_HM_LOG_ENABLED;
    else
        xmcTab.hpv.hmTab[hmEvent].log=XM_HM_LOG_DISABLED;
    AddLine(&xmcTab.hpv.hmTab[hmEvent], node->line);
}

static void IoRestrHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].noIoPorts++;
    xmcTab.noIoPorts++;
#if defined(CONFIG_IA32)
    PrintNoLine(node->line);
    ShowErrorMsgAndExit("Restricted ports are not supported by this architecture");
#elif defined(CONFIG_SPARCV8)
    ioPortTab=realloc(ioPortTab, sizeof(struct xmcIoPort)*xmcTab.noIoPorts);
    memset(&ioPortTab[CURRENT_IOPORT], 0, sizeof(struct xmcIoPort));
    ioPortTab[CURRENT_IOPORT].restrictedIoPort.mask=XM_DEFAULT_RESTRICTED_IOPORT_MASK;
    ioPortTab[CURRENT_IOPORT].type=XM_RESTRICTED_IOPORT;
    AddLine(&ioPortTab[CURRENT_IOPORT].type, node->line);
#endif
}

static void AddressIoRestrAttrHandler(xmlNodePtr node, const xmlChar *val) {
#if defined(CONFIG_IA32)
#elif defined(CONFIG_SPARCV8)
    ioPortTab[CURRENT_IOPORT].restrictedIoPort.address=strtoul((char *)val, 0, 16);
    AddLine(&ioPortTab[CURRENT_IOPORT].restrictedIoPort.address, node->line);
#endif
}

static void MaskIoRestrAttrHandler(xmlNodePtr node, const xmlChar *val) {
#if defined(CONFIG_IA32)
#elif defined(CONFIG_SPARCV8)
    ioPortTab[CURRENT_IOPORT].restrictedIoPort.mask=strtoul((char *)val, 0, 16);
    AddLine(&ioPortTab[CURRENT_IOPORT].restrictedIoPort.mask, node->line);
#endif
}

static void IoPortsHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].ioPortsOffset=xmcTab.noIoPorts;
#if defined(CONFIG_IA32)
    xmcTab.noIoPorts++;
    ioPortTab=realloc(ioPortTab, sizeof(struct xmcIoPort)*xmcTab.noIoPorts);
    memset(&ioPortTab[CURRENT_IOPORT], 0xFF, sizeof(struct xmcIoPort));
#endif
}

static unsigned long ioBase;

static void IoRangeHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].noIoPorts++;
#if defined(CONFIG_IA32)
#elif defined(CONFIG_SPARCV8)
    xmcTab.noIoPorts++;
    ioPortTab=realloc(ioPortTab, sizeof(struct xmcIoPort)*xmcTab.noIoPorts);
    memset(&ioPortTab[CURRENT_IOPORT], 0, sizeof(struct xmcIoPort));
    ioPortTab[CURRENT_IOPORT].type=XM_IOPORT_RANGE;
    AddLine(&ioPortTab[CURRENT_IOPORT].type, node->line);
#endif
}

static void BaseIoRangeAttrHandler(xmlNodePtr node, const xmlChar *val) {
#if defined(CONFIG_IA32)
    ioBase=strtoul((char *)val,0, 16);
#elif defined(CONFIG_SPARCV8)
    ioPortTab[CURRENT_IOPORT].ioPortRange.base=strtoul((char *)val,0, 16);
    AddLine(&ioPortTab[CURRENT_IOPORT].ioPortRange.base, node->line);
#endif
}

static void NoPortsIoRangeAttrHandler(xmlNodePtr node, const xmlChar *val) {
#if defined(CONFIG_IA32)
    if ((ioBase+strtoul((char *)val,0, 10))>65536) {
	PrintNoLine(node->line);
	ShowErrorMsgAndExit("IA32 only defines 65536 IOports\n");
    }
	
    BitmapClearBits(ioPortTab[CURRENT_IOPORT].map, ioBase, strtoul((char *)val,0, 10));    
#elif defined(CONFIG_SPARCV8)
    ioPortTab[CURRENT_IOPORT].ioPortRange.noPorts=strtoul((char *)val,0, 10);
    AddLine(&ioPortTab[CURRENT_IOPORT].ioPortRange.noPorts, node->line);
#endif
}

static void LineIrqAttrHandler(xmlNodePtr node, const xmlChar *val) {
    char *tmp, *tmp1;
    int line;

    for (tmp=(char *)val, tmp1=strstr(tmp, " "); tmp; tmp=((tmp1)?tmp1+1:0), tmp1=strstr(tmp, " ")) {
        if (tmp1) *tmp1=0;

	line=atoi(tmp);
	if (xmcTab.hpv.hwIrqTab[line].owner!=XM_IRQ_NO_OWNER) {
	    PrintNoLine(node->line);
	    ShowErrorMsgAndExit("Hw irq line \"%d\" already allocated to partition \"%d\"", line, xmcTab.hpv.hwIrqTab[line].owner);
	}
	xmcTab.hpv.hwIrqTab[line].owner=CURRENT_PARTITION;
	
	AddLine(&xmcTab.hpv.hwIrqTab[line].owner, node->line);
    }
}

static void PortTableHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].commPortsOffset=xmcTab.noCommPorts;
}

static void PortPortTableHandler(xmlNodePtr node) {
    partitionTab[CURRENT_PARTITION].noPorts++;
    xmcTab.noCommPorts++;
    commPorts=realloc(commPorts, xmcTab.noCommPorts*sizeof(struct xmcCommPort));
    memset(&commPorts[CURRENT_COMMPORT], 0, sizeof(struct xmcCommPort));
    commPorts[CURRENT_COMMPORT].channelId=XM_NULL_CHANNEL;
}

static void NamePortAttrHandler(xmlNodePtr node, const xmlChar *val) {
    commPorts[CURRENT_COMMPORT].nameOffset=xmcTab.stringTabLength;
    xmcTab.stringTabLength+=(strlen((char *)val)+1);
    stringTab=realloc(stringTab, xmcTab.stringTabLength);
    strcpy(&stringTab[commPorts[CURRENT_COMMPORT].nameOffset], (char *)val);
    AddLine(&stringTab[commPorts[CURRENT_COMMPORT].nameOffset], node->line);
}

static void DirectionPortAttrHandler(xmlNodePtr node, const xmlChar *val) {
    if(!xmlStrcasecmp(val, (xmlChar *)"source")) {
        commPorts[CURRENT_COMMPORT].direction=XM_SOURCE_PORT;
    } else if(!xmlStrcasecmp(val, (xmlChar *)"destination")) {
        commPorts[CURRENT_COMMPORT].direction=XM_DESTINATION_PORT;	
    }
    AddLine(&commPorts[CURRENT_COMMPORT].direction, node->line);
}

static void TypePortAttrHandler(xmlNodePtr node, const xmlChar *val) {
    if(!xmlStrcasecmp(val, (xmlChar *)"sampling")) {
	commPorts[CURRENT_COMMPORT].type=XM_SAMPLING_PORT;
    } else if(!xmlStrcasecmp(val, (xmlChar *)"queuing")) {
        commPorts[CURRENT_COMMPORT].type=XM_QUEUING_PORT;
    }
    AddLine(&commPorts[CURRENT_COMMPORT].type, node->line);
}

static void SourceChannelHandler(xmlNodePtr node) {
    linkTab[CURRENT_COMM_CHANNEL].noLinks++;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo=realloc(linkTab[CURRENT_COMM_CHANNEL].partitionInfo, linkTab[CURRENT_COMM_CHANNEL].noLinks*sizeof(struct partitionInfo));
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionName[0]=0;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].portName[0]=0;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].type=1;
}

static void DestinationChannelHandler(xmlNodePtr node) {
    linkTab[CURRENT_COMM_CHANNEL].noLinks++;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo=realloc(linkTab[CURRENT_COMM_CHANNEL].partitionInfo, linkTab[CURRENT_COMM_CHANNEL].noLinks*sizeof(struct partitionInfo));
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionName[0]=0;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].portName[0]=0;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].type=2;
}

static void PortNameSrcDstChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    strncpy(linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].portName, (char *)val, CONFIG_ID_STRING_LENGTH);
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].portName[CONFIG_ID_STRING_LENGTH-1]=0x0;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].portNameLine=node->line;
    //AddLine(linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].portName, node->line);
}

static void PartNameSrcDstChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    strncpy(linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionName, (char *)val, CONFIG_ID_STRING_LENGTH);
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionName[CONFIG_ID_STRING_LENGTH-1]=0x0;
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partNameLine=node->line;
    //AddLine(linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionName, node->line);
}

static void PartIdSrcDstChannelAttrHandler(xmlNodePtr node, const xmlChar *val) {
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionId=atoi((char *)val);
    linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partIdLine=node->line;
    //AddLine(&linkTab[CURRENT_COMM_CHANNEL].partitionInfo[linkTab[CURRENT_COMM_CHANNEL].noLinks-1].partitionId, node->line);
}

static void CpuHandler(xmlNodePtr node) {
    xmcTab.hpv.noCpus++;
}

static void IdCpuAttrHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.cpuTab[CURRENT_CPU].id=atoi((char *)val);
    AddLine(&xmcTab.hpv.cpuTab[CURRENT_CPU].id, node->line);
}

static void FreqCpuAttrHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.cpuTab[CURRENT_CPU].freq=FreqStr2Freq((char *)val);
    AddLine(&xmcTab.hpv.cpuTab[CURRENT_CPU].freq, node->line);
}

static void CyclicPlanSchedHpvHandler(xmlNodePtr node) {
    xmcTab.hpv.cpuTab[CURRENT_CPU].schedPolicy=XM_SCHED_CYCLIC;
    xmcTab.hpv.cpuTab[CURRENT_CPU].schedParams.cyclic.schedCyclicPlansOffset=xmcTab.noSchedCyclicPlans;
    AddLine(&xmcTab.hpv.cpuTab[CURRENT_CPU].schedPolicy, node->line);
}

static void PlanCyclicPlanHandler(xmlNodePtr node) {
    xmcTab.noSchedCyclicPlans++;
    xmcTab.hpv.cpuTab[CURRENT_CPU].schedParams.cyclic.noSchedCyclicPlans++;
    schedCyclicPlanTab=realloc(schedCyclicPlanTab, xmcTab.noSchedCyclicPlans*sizeof(struct xmcSchedCyclicPlan));
    memset(&schedCyclicPlanTab[CURRENT_HPV_SCHED_PLAN], 0, sizeof(struct xmcSchedCyclicPlan));
    schedCyclicPlanTab[CURRENT_HPV_SCHED_PLAN].slotsOffset=xmcTab.noSchedCyclicSlots;
}

static void IdCyclicPlanSchedHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    schedCyclicPlanTab[CURRENT_HPV_SCHED_PLAN].id=atoi((char *)val);
/*
    strncpy(xmcTab.hpv.cpuTab[CURRENT_CPU].schedParams.cyclic.planTab[CURRENT_HPV_SCHED_PLAN].name, (char *)val, CONFIG_ID_STRING_LENGTH);
    xmcTab.hpv.cpuTab[CURRENT_CPU].schedParams.cyclic.planTab[CURRENT_HPV_SCHED_PLAN].name[CONFIG_ID_STRING_LENGTH-1]=0x0;
*/
}

static void MajorFrameCyclicPlanSchedHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    schedCyclicPlanTab[CURRENT_HPV_SCHED_PLAN].majorFrame=TimeStr2Time((char *)val);
    AddLine(&schedCyclicPlanTab[CURRENT_HPV_SCHED_PLAN].majorFrame, node->line);
}

static void SlotPlanHandler(xmlNodePtr node) {
    schedCyclicPlanTab[CURRENT_HPV_SCHED_PLAN].noSlots++;
    xmcTab.noSchedCyclicSlots++;
    schedCyclicSlotTab=realloc(schedCyclicSlotTab, xmcTab.noSchedCyclicSlots*sizeof(struct xmcSchedCyclicSlot));
    memset(&schedCyclicSlotTab[CURRENT_CYCLICSLOT], 0, sizeof(struct xmcSchedCyclicSlot));
}

static void StartSlotSchedHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    schedCyclicSlotTab[CURRENT_CYCLICSLOT].sExec=TimeStr2Time((char *)val);
    AddLine(&schedCyclicSlotTab[CURRENT_CYCLICSLOT].sExec, node->line);
}

static void DurationSlotSchedHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    schedCyclicSlotTab[CURRENT_CYCLICSLOT].eExec=TimeStr2Time((char *)val)+schedCyclicSlotTab[CURRENT_CYCLICSLOT].sExec;
    AddLine(&schedCyclicSlotTab[CURRENT_CYCLICSLOT].eExec, node->line);
}

static void PartitionIdSlotSchedHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    schedCyclicSlotTab[CURRENT_CYCLICSLOT].partitionId=atoi((char *)val);
    AddLine(&schedCyclicSlotTab[CURRENT_CYCLICSLOT].partitionId, node->line);
}

static void IdSlotSchedHpvAttrHandler(xmlNodePtr node, const xmlChar *val) {
    schedCyclicSlotTab[CURRENT_CYCLICSLOT].id=strtoul((char *)val,0, 16);
    AddLine(&schedCyclicSlotTab[CURRENT_CYCLICSLOT].id, node->line);
}

static void RegionMemLayoutHandler(xmlNodePtr node) {
    xmcTab.noRegions++;
    memRegTab=realloc(memRegTab, xmcTab.noRegions*sizeof(struct xmcMemoryRegion));
    memset(&memRegTab[CURRENT_REGION], 0, sizeof(struct xmcMemoryRegion));
}

static void StartRegionAttrHandler(xmlNodePtr node, const xmlChar *val) {
    memRegTab[CURRENT_REGION].startAddr=strtoul((char *)val, 0, 16);
    AddLine(&memRegTab[CURRENT_REGION].startAddr, node->line);
}

static void SizeRegionAttrHandler(xmlNodePtr node, const xmlChar *val) {
    memRegTab[CURRENT_REGION].size=SizeStr2Size((char *)val);
    AddMemoryRegion(memRegTab[CURRENT_REGION].startAddr, memRegTab[CURRENT_REGION].startAddr+memRegTab[CURRENT_REGION].size-1, GetLine(&memRegTab[CURRENT_REGION].startAddr), node->line);
    AddLine(&memRegTab[CURRENT_REGION].size, node->line);
}

static void DevTracePartHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].trace.dev.id=InsertDevAsoc((char *)val);
    AddLine(&partitionTab[CURRENT_PARTITION].trace.dev, node->line);
}

static void BitmapTracePartHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].trace.bitmap=strtoul((char *)val,0, 16);
    AddLine(&partitionTab[CURRENT_PARTITION].trace.bitmap, node->line);
}

static void DevTraceHpvHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.trace.dev.id=InsertDevAsoc((char *)val);
    AddLine(&xmcTab.hpv.trace.dev, node->line);
}

static void BitmapTraceHpvHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.trace.bitmap=strtoul((char *)val,0, 16);
    AddLine(&xmcTab.hpv.trace.bitmap, node->line);
}

static void ConsoleDevHpvHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.consoleDev.id=InsertDevAsoc((char *)val);
    AddLine(& xmcTab.hpv.consoleDev, node->line);
}

static void HmDevHpvHandler(xmlNodePtr node, const xmlChar *val) {
    xmcTab.hpv.hmDev.id=InsertDevAsoc((char *)val);
    AddLine(&xmcTab.hpv.hmDev, node->line);
}

static void ConsoleDevPartHandler(xmlNodePtr node, const xmlChar *val) {
    partitionTab[CURRENT_PARTITION].consoleDev.id=InsertDevAsoc((char *)val);
    AddLine(&partitionTab[CURRENT_PARTITION].consoleDev, node->line);
}

// ATTRIBUTES
static struct attrXml maxNoMsgsLenSChannelAttr={(xmlChar *)"maxMessageLength", MaxMsgLenSChannelAttrHandler};

static struct attrXml maxNoMsgsLenQChannelAttr={(xmlChar *)"maxMessageLength", MaxMsgLenQChannelAttrHandler};

static struct attrXml maxNoMsgQChannelAttr={(xmlChar *)"maxNoMessages", MaxNoMsgsQChannelAttrHandler};

static struct attrXml validPeriodChannelAttr={(xmlChar *)"validPeriod", ValidPeriodChannelAttrHandler};

static struct attrXml periodTempReqAttr={(xmlChar *)"period", PeriodTempReqAttrHandler};
static struct attrXml durationTempReqAttr={(xmlChar *)"duration", DurationTempReqAttrHandler};

static struct attrXml idPartAttr={(xmlChar *)"id", IdPartAttrHandler};
static struct attrXml namePartAttr={(xmlChar *)"name", NamePartAttrHandler};
static struct attrXml processorPartAttr={(xmlChar *)"processor", ProcessorPartAttrHandler};
/*static struct attrXml svPartAttr={(xmlChar *)"supervisor", SvPartAttrHandler};
  static struct attrXml bootPartAttr={(xmlChar *)"boot", BootPartAttrHandler};*/
static struct attrXml flagsPartAttr={(xmlChar *)"flags", FlagsPartAttrHandler};
static struct attrXml fpPartAttr={(xmlChar *)"fp", FPPartAttrHandler};
static struct attrXml loadPhysAddrPartAttr={(xmlChar *)"loadPhysAddr", LoadPhysAddrPartAttrHandler};
static struct attrXml loadPhysAddrHpvAttr={(xmlChar *)"loadPhysAddr", LoadPhysAddrHpvAttrHandler};
static struct attrXml bootEntryPartAttr={(xmlChar *)"headerOffset", BootEntryPartAttrHandler};
static struct attrXml imgIdPartAttr={(xmlChar *)"imageId", ImgIdPartAttrHandler};

static struct attrXml versionSDAttr={(xmlChar *)"version", VersionSDAttrHandler};
static struct attrXml nameSDAttr={(xmlChar *)"name", NameSDAttrHandler};

static struct attrXml startMemAreaPartAttr={(xmlChar *)"start", StartMemAreaPartAttrHandler};

static struct attrXml sizeMemAreaPartAttr={(xmlChar *)"size", SizeMemAreaPartAttrHandler};

static struct attrXml flagsMemAreaPartAttr={(xmlChar *)"flags", FlagsMemAreaPartAttrHandler};

static struct attrXml startMemAreaHpvAttr={(xmlChar *)"start", StartMemAreaHpvAttrHandler};

static struct attrXml sizeMemAreaHpvAttr={(xmlChar *)"size", SizeMemAreaHpvAttrHandler};

static struct attrXml flagsMemAreaHpvAttr={(xmlChar *)"flags", FlagsMemAreaHpvAttrHandler};

static struct attrXml startMemAreaRswAttr={(xmlChar *)"start", StartMemAreaRswAttrHandler};

static struct attrXml sizeMemAreaRswAttr={(xmlChar *)"size", SizeMemAreaRswAttrHandler};

static struct attrXml flagsMemAreaRswAttr={(xmlChar *)"flags", FlagsMemAreaRswAttrHandler};

static struct attrXml entryPointRswAttr={(xmlChar *)"entryPoint", EntryPointRswAttrHandler};

static struct attrXml nameHmEventAttr={(xmlChar *)"name", NameHmEventAttrHandler};
static struct attrXml actionHmEventPartAttr={(xmlChar *)"action", ActionHmEventPartAttrHandler};
static struct attrXml logHmEventPartAttr={(xmlChar *)"log", LogHmEventPartAttrHandler};
static struct attrXml actionHmEventHpvAttr={(xmlChar *)"action", ActionHmEventHpvAttrHandler};
static struct attrXml logHmEventHpvAttr={(xmlChar *)"log", LogHmEventHpvAttrHandler};
static struct attrXml addressIoRestrAttr={(xmlChar *)"address", AddressIoRestrAttrHandler};
static struct attrXml maskIoRestrAttr={(xmlChar *)"mask", MaskIoRestrAttrHandler};
static struct attrXml baseIoRangeAttr={(xmlChar *)"base", BaseIoRangeAttrHandler};
static struct attrXml noPortsIoRangeAttr={(xmlChar *)"noPorts", NoPortsIoRangeAttrHandler};
static struct attrXml lineIrqAttr={(xmlChar *)"line", LineIrqAttrHandler};
static struct attrXml namePortAttr={(xmlChar *)"name", NamePortAttrHandler};
static struct attrXml directionPortAttr={(xmlChar *)"direction", DirectionPortAttrHandler};
static struct attrXml typePortAttr={(xmlChar *)"type", TypePortAttrHandler};
static struct attrXml idCpuAttr={(xmlChar *)"id", IdCpuAttrHandler};
static struct attrXml freqCpuAttr={(xmlChar *)"frequency", FreqCpuAttrHandler};
static struct attrXml featCpuAttr={(xmlChar *)"features", FeatCpuAttrHandler};
static struct attrXml idCyclicPlanSchedHpvAttr={(xmlChar *)"id", IdCyclicPlanSchedHpvAttrHandler};
static struct attrXml majorFrameCyclicPlanSchedHpvAttr={(xmlChar *)"majorFrame", MajorFrameCyclicPlanSchedHpvAttrHandler};

/*static struct attrXml nameSlotSchedHpvAttr={(xmlChar *)"name",
 * NameSlotSchedHpvAttrHandler};*/
static struct attrXml startSlotSchedHpvAttr={(xmlChar *)"start", StartSlotSchedHpvAttrHandler};
static struct attrXml durationSlotSchedHpvAttr={(xmlChar *)"duration", DurationSlotSchedHpvAttrHandler};
static struct attrXml partitionIdSlotSchedHpvAttr={(xmlChar *)"partitionId", PartitionIdSlotSchedHpvAttrHandler};
static struct attrXml idSlotSchedHpvAttr={(xmlChar *)"id", IdSlotSchedHpvAttrHandler};
static struct attrXml startRegionAttr={(xmlChar *)"start", StartRegionAttrHandler};
static struct attrXml sizeRegionAttr={(xmlChar *)"size", SizeRegionAttrHandler};
static struct attrXml partIdSrcDstChannelAttr={(xmlChar *)"partitionId", PartIdSrcDstChannelAttrHandler};
static struct attrXml partNameSrcDstChannelAttr={(xmlChar *)"partitionName", PartNameSrcDstChannelAttrHandler};
static struct attrXml portNameSrcDstChannelAttr={(xmlChar *)"portName", PortNameSrcDstChannelAttrHandler};
static struct attrXml devTracePartAttr={(xmlChar *)"device", DevTracePartHandler};
static struct attrXml bitmapTracePartAttr={(xmlChar *)"bitmask", BitmapTracePartHandler};
static struct attrXml devTraceHpvAttr={(xmlChar *)"device", DevTraceHpvHandler};
static struct attrXml bitmapTraceHpvAttr={(xmlChar *)"bitmask", BitmapTraceHpvHandler};

static struct attrXml consoleDevHpvAttr={(xmlChar *)"console", ConsoleDevHpvHandler};

static struct attrXml consoleDevPartAttr={(xmlChar *)"console", ConsoleDevPartHandler};

static struct attrXml hmDevHpvAttr={(xmlChar *)"healthMonitoringDevice", HmDevHpvHandler};

static struct attrXml typeRegionAttr={(xmlChar *)"type", 0};

// NODES
static struct nodeXml eventHmPartNode={(xmlChar *)"Event", 0, .attrList={&nameHmEventAttr, &actionHmEventPartAttr, &logHmEventPartAttr, 0}, .children={0}};

static struct nodeXml eventHmHpvNode={(xmlChar *)"Event", 0, .attrList={&nameHmEventAttr, &actionHmEventHpvAttr, &logHmEventHpvAttr, 0}, .children={0}};

static struct nodeXml slotPlanSchedHpvNode={(xmlChar *)"Slot", SlotPlanHandler, .attrList={&startSlotSchedHpvAttr, &durationSlotSchedHpvAttr, &partitionIdSlotSchedHpvAttr, &idSlotSchedHpvAttr, 0}, .children={0}};

static struct nodeXml planSchedHpvNode={(xmlChar *)"Plan", PlanCyclicPlanHandler, .attrList={&idCyclicPlanSchedHpvAttr, &majorFrameCyclicPlanSchedHpvAttr, 0}, .children={&slotPlanSchedHpvNode, 0}};

static struct nodeXml cyclicPlanSchedHpvNode={(xmlChar *)"CyclicPlanTable", CyclicPlanSchedHpvHandler, .attrList={0}, .children={&planSchedHpvNode, 0}};

static struct nodeXml schedCpuHpvNode={(xmlChar *)"Sched", 0, .attrList={0}, .children={&cyclicPlanSchedHpvNode, 0}};

static struct nodeXml memAreaHpvNode={(xmlChar *)"Area", MemAreaHpvHandler, .attrList={&startMemAreaHpvAttr, &sizeMemAreaHpvAttr, &flagsMemAreaHpvAttr, 0}, .children={0}};

static struct nodeXml memAreaRswNode={(xmlChar *)"Area", MemAreaRswHandler, .attrList={&startMemAreaRswAttr, &sizeMemAreaRswAttr, &flagsMemAreaRswAttr, 0}, .children={0}};

static struct nodeXml memAreaPartNode={(xmlChar *)"Area", MemAreaPartHandler, .attrList={&startMemAreaPartAttr, &sizeMemAreaPartAttr, &flagsMemAreaPartAttr, 0}, .children={0}};

static struct nodeXml physMemAreasPartNode={(xmlChar *)"PhysicalMemoryAreas", PhysMemAreasPartHandler, .attrList={0}, .children={&memAreaPartNode, 0}};

static struct nodeXml processorNode={(xmlChar *)"Processor", CpuHandler, .attrList={&idCpuAttr, &freqCpuAttr, &featCpuAttr, 0}, .children={&schedCpuHpvNode, 0}};

static struct nodeXml regionMemLayoutNode={(xmlChar *)"Region", RegionMemLayoutHandler, .attrList={&startRegionAttr, &sizeRegionAttr, &typeRegionAttr, 0}, .children={0}};

static struct nodeXml memLayoutNode={(xmlChar *)"MemoryLayout", 0, .attrList={0}, .children={&regionMemLayoutNode, 0}};

static struct nodeXml hwDescrNode={(xmlChar *)"HwDescription", 0, .attrList={0}, .children={&processorNode, &memLayoutNode, 0}};

static struct nodeXml physMemAreasHpvNode={(xmlChar *)"PhysicalMemoryAreas", PhysMemAreasHpvHandler, .attrList={0}, .children={&memAreaHpvNode, 0}};

static struct nodeXml physMemAreasRswNode={(xmlChar *)"PhysicalMemoryAreas", PhysMemAreasRswHandler, .attrList={0}, .children={&memAreaRswNode, 0}};

static struct nodeXml tempReqNode={(xmlChar *)"TemporalRequirements", 0, .attrList={&periodTempReqAttr, &durationTempReqAttr, 0}, .children={0}};

static struct nodeXml ioRangeNode={(xmlChar *)"Range",  IoRangeHandler, .attrList={&baseIoRangeAttr, &noPortsIoRangeAttr, 0}, .children={0}};

static struct nodeXml ioRestrNode={(xmlChar *)"Restricted",  IoRestrHandler, .attrList={&addressIoRestrAttr, &maskIoRestrAttr, 0}, .children={0}};

static struct nodeXml tracePartNode={(xmlChar *)"Trace", 0, .attrList={&devTracePartAttr, &bitmapTracePartAttr, 0}, .children={0}};

static struct nodeXml traceHpvNode={(xmlChar *)"Trace", 0, .attrList={&devTraceHpvAttr, &bitmapTraceHpvAttr, 0}, .children={0}};

static struct nodeXml ioPortsNode={(xmlChar *)"IoPorts", IoPortsHandler, .attrList={0}, .children={&ioRangeNode, &ioRestrNode, 0}};

static struct nodeXml interruptsNode={(xmlChar *)"Interrupts", 0, .attrList={&lineIrqAttr, 0}, .children={0}};

static struct nodeXml hmPartNode={(xmlChar *)"HealthMonitoring", 0, .attrList={0}, .children={&eventHmPartNode, 0}};

static struct nodeXml hmHpvNode={(xmlChar *)"HealthMonitoring", 0, .attrList={0}, .children={&eventHmHpvNode, 0}};

static struct nodeXml hwResourcesNode={(xmlChar *)"HwResources", 0, .attrList={0}, .children={&ioPortsNode, &interruptsNode, 0}};

static struct nodeXml portPortTablePartNode={(xmlChar *)"Port", PortPortTableHandler, .attrList={&namePortAttr, &directionPortAttr, &typePortAttr, 0}, .children={0}};

static struct nodeXml portTablePartNode={(xmlChar *)"PortTable", PortTableHandler, .attrList={0}, .children={&portPortTablePartNode, 0}};

static struct nodeXml rswNode={(xmlChar *)"ResidentSw", 0, .attrList={&entryPointRswAttr, 0}, .children={&physMemAreasRswNode, 0}};

static struct nodeXml xmHypervisorNode={(xmlChar *)"XMHypervisor", 0, .attrList={&consoleDevHpvAttr, &hmDevHpvAttr, &loadPhysAddrHpvAttr, 0}, .children={&physMemAreasHpvNode, &hwDescrNode, &hmHpvNode, &traceHpvNode, 0}};

static struct nodeXml partitionNode={(xmlChar *)"Partition", PartitionHandler, .attrList={&idPartAttr, &namePartAttr, &processorPartAttr, &loadPhysAddrPartAttr, &flagsPartAttr, &bootEntryPartAttr, &fpPartAttr, &imgIdPartAttr, &consoleDevPartAttr, 0}, .children={&physMemAreasPartNode, &tempReqNode, &tempReqNode, &hmPartNode, &hwResourcesNode, &portTablePartNode, &tracePartNode, 0}};

static struct nodeXml partitionTableNode={(xmlChar *)"PartitionTable", 0, .attrList={0}, .children={&partitionNode, 0}};

static struct nodeXml sourceChannelNode={(xmlChar *)"Source",  SourceChannelHandler, .attrList={&partIdSrcDstChannelAttr, &partNameSrcDstChannelAttr, &portNameSrcDstChannelAttr, 0}, .children={0}};

static struct nodeXml destinationChannelNode={(xmlChar *)"Destination",  DestinationChannelHandler, .attrList={&partIdSrcDstChannelAttr, &partNameSrcDstChannelAttr, &portNameSrcDstChannelAttr, 0}, .children={0}};

static struct nodeXml samplingChannelNode={(xmlChar *)"SamplingChannel",  SamplingChannelHandler, .attrList={&maxNoMsgsLenSChannelAttr, &validPeriodChannelAttr, 0}, .children={&sourceChannelNode, &destinationChannelNode, 0}};

static struct nodeXml queuingChannelNode={(xmlChar *)"QueuingChannel",  QueuingChannelHandler, .attrList={&maxNoMsgsLenQChannelAttr, &maxNoMsgQChannelAttr, &validPeriodChannelAttr, 0}, .children={&sourceChannelNode, &destinationChannelNode, 0}};

static struct nodeXml channelsNode={(xmlChar *)"Channels", 0, .attrList={0}, .children={&samplingChannelNode, &queuingChannelNode, 0}};

static struct nodeXml devicesNode={(xmlChar *)"Devices", 0, .attrList={0}, .children={0}};

static struct nodeXml systemDescriptionNode={(xmlChar *)"SystemDescription", 0, .attrList={&versionSDAttr, &nameSDAttr, 0}, .children={&xmHypervisorNode, &rswNode, &partitionTableNode, &channelsNode, &devicesNode, 0}};

struct nodeXml *rootHandlers[MAX_CHILDREN_PER_NODE]={&systemDescriptionNode, 0};
