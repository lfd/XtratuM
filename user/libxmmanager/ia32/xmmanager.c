/*
 * $FILE: xmmanager-ia32.c
 *
 * XM Partition Manager: ia32 frontend
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include "xm.h"
#include "xmmanager.h"
#include <std_c.h>

int UartWrite(char *s, int n);
int UartRead(char *s, int n);
int UartInit(void);

static struct XmManagerDevice_t uartdev = {
    .flags = DEVICE_FLAG_COOKED,

    .init = UartInit,
    .read = UartRead,
    .write = UartWrite,
};

void PartitionMain(void) {
    while(1){
        XmManager(&uartdev);
    }
}

