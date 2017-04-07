/*
 * $FILE: xmcparser.h
 *
 * XtratuM's XML configuration parser to C
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XMC_PARSER_H_
#define _XMC_PARSER_H_

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlschemas.h>
#include <xm_inc/xmconf.h>

struct partitionInfo {
    char portName[CONFIG_ID_STRING_LENGTH];
    int portNameLine;
    char partitionName[CONFIG_ID_STRING_LENGTH];
    int partNameLine;
    int partitionId;
    int partIdLine;
// 0 -> source, 1 -> destination
    int type;
};

struct linkTab {
    struct partitionInfo *partitionInfo;
    int noLinks;
};

extern struct linkTab *linkTab;

#define MAX_ATTR_PER_NODE 20
#define MAX_CHILDREN_PER_NODE 10

struct attrXml {
    const xmlChar *name;
    void (*handler)(xmlNodePtr, const xmlChar *);
};

struct nodeXml {
    const xmlChar *name;
    void (*handler)(xmlNodePtr);
    struct attrXml *attrList[MAX_ATTR_PER_NODE];
    struct nodeXml *children[MAX_CHILDREN_PER_NODE];
};

struct nodeXml *rootHandlers[MAX_CHILDREN_PER_NODE];

extern char *xmlFile;
extern struct xmc xmcTab;
extern struct xmcPartition *partitionTab;
extern struct xmcMemoryRegion *memRegTab;
extern struct xmcCommChannel *commChannelTab;
extern struct xmcMemoryArea *physMemAreaTab;
extern struct xmcCommPort *commPorts;
extern struct xmcIoPort *ioPortTab;
extern struct xmcSchedCyclicSlot *schedCyclicSlotTab;
extern struct xmcSchedCyclicPlan *schedCyclicPlanTab;
extern char *stringTab;

extern void PrintLine(void *addr);
extern void AddLine(void *addr, int line);
extern void PrintNoLine(int line);
extern int GetLine(void *addr);
extern void ShowErrorMsgAndExit(const char *msg, ...);
extern unsigned long TimeStr2Time(char *input);
extern unsigned long SizeStr2Size(char *input);
extern unsigned long FreqStr2Freq(char *input);
extern void InitXMCDefaults(struct xmc *xmcTab);
extern int ProcessXMC(struct xmc *xmcTab);
extern void PrintXMC(struct xmc *xmcTab, FILE *outFile);
extern int InsertChildNode(struct nodeXml *root, struct nodeXml *node);
extern struct nodeXml *LookUpNode(struct nodeXml *handlers[], xmlChar *name);
extern struct attrXml *LookUpAttribute(struct nodeXml *handlers[], xmlChar *node, xmlChar *attr);
extern void ArchInit(struct xmc *xmcTab);

#define BITS_PER_U32 (32)

#define LOG2_32 (5)
#define MOD_U32_MASK ((1<<LOG2_32)-1)

static inline int BitmapClearBits(unsigned int *b, unsigned int bp, int nb) {
    unsigned int b_entry=bp>>LOG2_32, bit=bp&MOD_U32_MASK;
    unsigned int mask=((((nb>=BITS_PER_U32)?~0:(1<<nb)-1))<<bit);
    //unsigned int tmp;

    //tmp=b[b_entry];
    b[b_entry++]&=~mask;
    nb-=(BITS_PER_U32-bit);

    while(nb>0) {
        mask=(((nb>=BITS_PER_U32)?~0:(1<<nb)-1));
        //tmp=b[b_entry];
        b[b_entry++]&=~mask;
        nb-=BITS_PER_U32;
    }

    return 0;
}

#define CURRENT_PARTITION (xmcTab.noPartitions-1)
#define CURRENT_COMM_CHANNEL (xmcTab.noCommChannels-1)
#define CURRENT_HPV_PHYSMEMAREA (xmcTab.hpv.noPhysicalMemoryAreas-1)
#define CURRENT_PART_PHYSMEMAREA (partitionTab[CURRENT_PARTITION].noPhysicalMemoryAreas-1)
#define CURRENT_RSW_PHYSMEMAREA (xmcTab.rsw.noPhysicalMemoryAreas-1)
#define CURRENT_PART_VIRTMEMAREA (partitionTab[CURRENT_PARTITION].noVirtualMemoryAreas-1)
#define CURRENT_PART_IOPORT (partitionTab[CURRENT_PARTITION].noIoPorts-1)
#define CURRENT_PART_COMMPORT (partitionTab[CURRENT_PARTITION].noPorts-1)
#define CURRENT_CPU (xmcTab.hpv.noCpus-1)
#define CURRENT_HPV_SCHED_PLAN (xmcTab.hpv.cpuTab[CURRENT_CPU].schedParams.cyclic.noSchedCyclicPlans-1)
#define CURRENT_HPV_PLAN_SLOT (xmcTab.noSchedCyclicSlots-1)
#define CURRENT_REGION (xmcTab.noRegions-1)
#define CURRENT_PHYSMEMAREA (xmcTab.noPhysicalMemoryAreas-1)
#define CURRENT_COMMPORT (xmcTab.noCommPorts-1)
#define CURRENT_IOPORT (xmcTab.noIoPorts-1)
#define CURRENT_CYCLICSLOT (xmcTab.noSchedCyclicSlots-1)
#endif
