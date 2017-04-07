/*
 * $FILE: memblock.c
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
#include "../constraints.h"
#include "../devices.h"
#include "../xml2c.h"

#ifdef CONFIG_DEV_MEMBLOCK

struct xmcMemBlock *memBlockTab=0;
#define CURRENT_MEMBLOCK (xmcTab.deviceTab.noMemBlocks-1)

static void StartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    memBlockTab[CURRENT_MEMBLOCK].startAddr=strtoul((char *)val,0, 16);
    AddLine(&memBlockTab[CURRENT_MEMBLOCK].startAddr, node->line);
}

static void SizeAttrHandler(xmlNodePtr node, const xmlChar *val) {  
    memBlockTab[CURRENT_MEMBLOCK].size=SizeStr2Size((char *)val);
    AddMemoryArea(memBlockTab[CURRENT_MEMBLOCK].startAddr, memBlockTab[CURRENT_MEMBLOCK].startAddr+memBlockTab[CURRENT_MEMBLOCK].size, GetLine(&memBlockTab[CURRENT_MEMBLOCK].startAddr), node->line);
    AddLine(&memBlockTab[CURRENT_MEMBLOCK].size, node->line);
}

static void NameAttrHandler(xmlNodePtr node, const xmlChar *val) {
    InsertDevice((char *)val, (xmDev_t){XM_DEV_LOGSTORAGE_ID, xmcTab.deviceTab.noMemBlocks-1});
}

static void MemBlockHandler(xmlNodePtr node) {
    xmcTab.deviceTab.noMemBlocks++;
    memBlockTab=realloc(memBlockTab, sizeof(struct xmcMemBlock)*xmcTab.deviceTab.noMemBlocks);
    memset(&memBlockTab[CURRENT_MEMBLOCK], 0, sizeof(struct xmcMemBlock));
}

static struct attrXml startAttr={(xmlChar *)"start", StartAttrHandler};
static struct attrXml sizeAttr={(xmlChar *)"size", SizeAttrHandler};
static struct attrXml nameAttr={(xmlChar *)"name", NameAttrHandler};

static struct nodeXml memBlockNode={(xmlChar *)"Block", &MemBlockHandler, .attrList={&nameAttr, &startAttr, &sizeAttr, 0}, .children={0}};

static struct nodeXml memBlockTabNode={(xmlChar *)"MemoryBlockTable", 0, .attrList={0}, .children={&memBlockNode, 0}};

static void MemBlock2c(struct xmc *xmcTab, FILE *outFile) {
    fprintf(outFile, ADDNTAB(2, ".memBlocksOffset = (xmAddress_t)xmcMemBlockTab,\n"));
    fprintf(outFile, ADDNTAB(2, ".noMemBlocks = %d,\n"), xmcTab->deviceTab.noMemBlocks);
}

static void MemBlockTab2c(struct xmc *xmcTab, FILE *outFile) {
    int e;
    fprintf(outFile, "struct xmcMemBlock xmcMemBlockTab[]={\n");
    
    for (e=0; e<xmcTab->deviceTab.noMemBlocks; e++) {
	fprintf(outFile, ADDNTAB(1, "[%d] = {\n"), e);
	fprintf(outFile, ADDNTAB(2, ".startAddr = 0x%x,\n"), memBlockTab[e].startAddr);
	fprintf(outFile, ADDNTAB(2, ".size = %d,\n"), memBlockTab[e].size);
	fprintf(outFile, ADDNTAB(1, "},\n"));
    }
    fprintf(outFile, "};\n\n");
}

void SetupMemBlock(void) {
    struct nodeXml *devicesNodeXml;
    
    if (!(devicesNodeXml=LookUpNode(rootHandlers, (xmlChar *)"Devices")))
        ShowErrorMsgAndExit("Devices node not found??");
 
    InsertChildNode(devicesNodeXml, &memBlockTabNode);
    InsertDevTab2cHandler(MemBlockTab2c);
    InsertDev2cHandler(MemBlock2c);
}

REGISTER_DEVICE(SetupMemBlock);
#endif
