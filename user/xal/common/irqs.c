/*
 * $FILE: irqs.c
 *
 * Generic traps' handler
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <stdio.h>
#include <irqs.h>

trapHandler_t trapHandlersTab[TRAPTAB_LENGTH];

static void UnexpectedTrap(trapCtxt_t *ctxt) {
    printf("[P%d] Unexpected trap 0x%x (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->irqNr, ctxt->pc);
}

xm_s32_t InstallTrapHandler(xm_s32_t trapNr, trapHandler_t handler) {
    if (trapNr<0||trapNr>TRAPTAB_LENGTH)
        return -1;

    if (handler)
        trapHandlersTab[trapNr]=handler;
    else
        trapHandlersTab[trapNr]=UnexpectedTrap;
    return 0;
}

void SetupIrqs(void) {
    xm_s32_t e;

    for (e=0; e<TRAPTAB_LENGTH; e++)
        trapHandlersTab[e]=UnexpectedTrap;

/*    for (e=0; e<XM_VT_EXT_MAX; e++) */
/*        XM_route_irq(XM_EXTIRQ_TYPE, e, 224+e);*/
}
