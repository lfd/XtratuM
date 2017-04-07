/*
 * $FILE: pc_uart.c
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

#ifdef CONFIG_DEV_PC_UART
#include <string.h>
#include "../xmcparser.h"
#include "../devices.h"
#include "../xml2c.h"

static void NamePcUartAttrHandler(xmlNodePtr node, const xmlChar *val) {
    InsertDevice((char *)val, (xmDev_t){XM_DEV_PC_UART_ID, 0});
}

static struct attrXml namePcUartAttr={(xmlChar *)"name", NamePcUartAttrHandler};

static struct nodeXml pcUartNode={(xmlChar *)"PcUart", 0, .attrList={&namePcUartAttr, 0}, .children={0}};

static void PcUart2c(struct xmc *xmcTab, FILE *outFile) {
    fprintf(outFile, ADDNTAB(2, ".pcUart = {},\n"));
}

void SetupPcUart(void) {
    struct nodeXml *devicesNodeXml;
    
    if (!(devicesNodeXml=LookUpNode(rootHandlers, (xmlChar *)"Devices")))
        ShowErrorMsgAndExit("Devices node not found??");
 
    InsertChildNode(devicesNodeXml, &pcUartNode);
    InsertDev2cHandler(PcUart2c);
}

REGISTER_DEVICE(SetupPcUart);

#endif
