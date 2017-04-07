/*
 * $FILE: devices.c
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
#include "xmcparser.h"
#include "devices.h"
#include "xml2c.h"

static struct devAsoc {
    char name[CONFIG_ID_STRING_LENGTH];
} *devAsocTab=0;

static int noDevAsoc=0;

static struct device {
    char name[CONFIG_ID_STRING_LENGTH];
    xmDev_t dev;
} *deviceTab=0;

static int noDevices=0;

int InsertDevAsoc(char *name) {
    xm_s32_t e;    
    for (e=0; e<noDevAsoc; e++) {
	if (!strcmp(name, devAsocTab[e].name)) {
	    return e;
	}
    }

    noDevAsoc++;
    devAsocTab=realloc(devAsocTab, noDevAsoc*sizeof(struct devAsoc));    
    strncpy(devAsocTab[noDevAsoc-1].name, name, CONFIG_ID_STRING_LENGTH);
    devAsocTab[noDevAsoc-1].name[CONFIG_ID_STRING_LENGTH-1]=0x0;
    return noDevAsoc-1;
}

char *GetDevAsocName(unsigned int id) {
    if ((id==XM_DEV_INVALID_ID)||(id>=noDevAsoc))
	return 0;

    return devAsocTab[id].name;
}

void InsertDevice(char *name, xmDev_t dev) {
    noDevices++;
    deviceTab=realloc(deviceTab, noDevices*sizeof(struct device));    
    strncpy(deviceTab[noDevices-1].name, name, CONFIG_ID_STRING_LENGTH);
    deviceTab[noDevices-1].name[CONFIG_ID_STRING_LENGTH-1]=0x0;
    deviceTab[noDevices-1].dev=dev;
}

void LookUpDevice(char *name, xmDev_t *dev) {
    int e;
    if (!dev)
	ShowErrorMsgAndExit("No device passed as argument\n");
	
    dev->id=XM_DEV_INVALID_ID;
    dev->subId=0;
    if (!name)
	return;
    
    for (e=0; e<noDevices; e++) {
	if (!strcmp(deviceTab[e].name, name)) {
	    *dev=deviceTab[e].dev;
	    return;
	}
    }
    ShowErrorMsgAndExit("Device \"%s\" is not associated to any existing device\n", name);
}

dev2cHandler_t *dev2cTab=0;
static int noDev2cHandlers=0;

void InsertDev2cHandler(dev2cHandler_t handler) {
    noDev2cHandlers++;
    dev2cTab=realloc(dev2cTab, noDev2cHandlers*sizeof(dev2cHandler_t));    
    dev2cTab[noDev2cHandlers-1]=handler;
}

void ExecDev2cHandlers(struct xmc *xmcTab, FILE *outFile) {
    int e;
    for (e=0; e<noDev2cHandlers; e++) {
	if (dev2cTab[e])
	    dev2cTab[e](xmcTab, outFile);
    }
}

dev2cHandler_t *devTab2cTab=0;
static int noDevTab2cHandlers=0;

void InsertDevTab2cHandler(dev2cHandler_t handler) {
    noDevTab2cHandlers++;
    devTab2cTab=realloc(devTab2cTab, noDevTab2cHandlers*sizeof(dev2cHandler_t));    
    devTab2cTab[noDevTab2cHandlers-1]=handler;
}

void ExecDevTab2cHandlers(struct xmc *xmcTab, FILE *outFile) {
    int e;
    for (e=0; e<noDevTab2cHandlers; e++) {
	if (devTab2cTab[e])
	    devTab2cTab[e](xmcTab, outFile);
    }
}
