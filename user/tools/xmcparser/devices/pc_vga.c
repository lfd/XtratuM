/*
 * $FILE: pc_vga.c
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

#ifdef CONFIG_DEV_PC_VGA
#include <string.h>
#include "../xmcparser.h"
#include "../devices.h"
#include "../xml2c.h"

static void NamePcVgaAttrHandler(xmlNodePtr node, const xmlChar *val) {
    InsertDevice((char *)val, (xmDev_t){XM_DEV_PC_VGA_ID, 0});
}

static struct attrXml namePcVgaAttr={(xmlChar *)"name", NamePcVgaAttrHandler};

static struct nodeXml pcVgaNode={(xmlChar *)"PcVga", 0, .attrList={&namePcVgaAttr, 0}, .children={0}};

static void PcVga2c(struct xmc *xmcTab, FILE *outFile) {
    fprintf(outFile, ADDNTAB(2, ".pcVga = {},\n"));
}

void SetupPcVga(void) {
    struct nodeXml *devicesNodeXml;
    
    if (!(devicesNodeXml=LookUpNode(rootHandlers, (xmlChar *)"Devices")))
        ShowErrorMsgAndExit("Devices node not found??");
 
    InsertChildNode(devicesNodeXml, &pcVgaNode);
    InsertDev2cHandler(PcVga2c);
}

REGISTER_DEVICE(SetupPcVga);

#endif
