/*
 * $FILE: hdr.c
 *
 * XtratuM Partition Image Header 
 *
 * $VERSION$
 *
 * Author: Salva Peiró <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spare.h"

#define SPARE_PORT_NAME "spreq"

extern xm_u32_t _sguest[];
extern xm_u32_t _eguest[];
extern struct xmPartitionHdr __xmPartitionHdr[];
xm_u8_t __customFile1[1024] __attribute__ ((section(".bss.noinit")));

struct xmImageHdr __xmImageHdr __attribute__ ((section(".text.init#allow,#exclude"))) = {
    .signature=XMEF_PARTITION_MAGIC,
    .xmAbiVersion=XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION),
    .xmApiVersion=XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION),
    .imageId = 0,
    .checksum = 0,
    .sAddr = (xmAddress_t) _sguest,
    .eAddr = (xmAddress_t) _eguest,
    .entry = {
        .defaultPartitionHdr = __xmPartitionHdr,
    },
    .noModules = 1,
    .moduleTab={
        [0]={
            .sAddr=(xmAddress_t)__customFile1,
            .size=sizeof(__customFile1),
        },
    },
};

static inline void * GetXmc(void) {
    extern struct xmImageHdr __xmImageHdr;
    struct xmc *xmc;
    xm_u32_t i, xmcSize;
    for (i=0; i < __xmImageHdr.noModules; i++)
    {
        xmc = (void*)__xmImageHdr.moduleTab[i].sAddr;
        xmcSize = __xmImageHdr.moduleTab[i].size;
        if (xmc && xmcSize && xmc->signature == XMC_SIGNATURE)
            return xmc;
    }
    return 0;
}

static char * GetString(const struct xmc *xmc, int offset)
{
    return (char *) ((xmAddress_t)xmc + (xmAddress_t) xmc->stringsOffset + (xmAddress_t) offset);
}

int InitChannels(struct partitionData * partitionData)
{
    char *portName;
    int i, j, noPorts = 0;
    const struct xmc *xmc = GetXmc();
    const struct xmcCommChannel *xmcCommChannelTab;
    const struct xmcCommPort *xmcCommPorts;

    ASSERT(xmc);
    xmcCommChannelTab = (struct xmcCommChannel *) ((xmAddress_t)xmc + xmc->commChannelTabOffset);
    xmcCommPorts = (struct xmcCommPort *) ((xmAddress_t)xmc + xmc->commPortsOffset);

    ASSERT(xmc->noCommChannels && xmc->noCommPorts);
    
    for (i = 0; i < xmc->noCommPorts; i++) {
        if (xmcCommPorts[i].type != XM_SAMPLING_PORT)
            continue;
        if (xmcCommPorts[i].direction != XM_DESTINATION_PORT)
            continue;
        portName = GetString(xmc, xmcCommPorts[i].nameOffset);
        if(strncmp(portName, SPARE_PORT_NAME, sizeof(SPARE_PORT_NAME)-1) != 0)
            continue;
        j = atoi(portName+sizeof(SPARE_PORT_NAME)-1);

        partitionData[j].partitionId = j;
        partitionData[j].portName = portName;
        partitionData[j].portSize = xmcCommChannelTab[xmcCommPorts[i].channelId].s.maxLength;
        partitionData[j].portDesc=0;
        noPorts++;
    }
    return noPorts;
}
