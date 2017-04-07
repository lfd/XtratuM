/*
 * $FILE: process_xml.c
 *
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <string.h>
#include "../xmcparser.h"

static void Ia32TypeRegionAttrHandler(xmlNodePtr node, const xmlChar *val) {
    if(!xmlStrcasecmp(val, (xmlChar *)"ram")) {
	memRegTab[CURRENT_REGION].flags|=XMC_REG_FLAG_PGTAB;
    } /*else if(!xmlStrcasecmp(val, (xmlChar *)"rom")) {
	
	}*/
    
    AddLine(&memRegTab[CURRENT_REGION].flags, node->line);
}

void FeatCpuAttrHandler(xmlNodePtr node, const xmlChar *val) {
    char *tmp, *tmp1;

    for (tmp=(char *)val, tmp1=strstr(tmp, " "); tmp; tmp=((tmp1)?tmp1+1:0), tmp1=strstr(tmp, " ")) {
        if (tmp1) *tmp1=0;
    }
    AddLine(&xmcTab.hpv.cpuTab[CURRENT_CPU].features, node->line);
}

void ArchInit(struct xmc *xmcTab) {
    struct nodeXml *hpvNodeXml;
    struct attrXml *typeRegAttrXml;
    
    if (!(hpvNodeXml=LookUpNode(rootHandlers, (xmlChar *)"XMHypervisor")))
    	ShowErrorMsgAndExit("XMHypervisor node not found??");
    if (!(typeRegAttrXml=LookUpAttribute(hpvNodeXml->children, (xmlChar *)"Region", (xmlChar *)"type")))
    	ShowErrorMsgAndExit("type attribute not found??");
    
    typeRegAttrXml->handler=Ia32TypeRegionAttrHandler;
}
