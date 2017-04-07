/*
 * $FILE: arch.c
 *
 * Architecture initialization functions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */
#include <irqs.h>
#include <stdio.h>
#include <xm.h>

#define GUEST_CS_SEL ((1<<3)|1)

typedef struct {
    xm_u32_t offsetLow:16,      /* offset 0..15 */
        selector:16,
        wordCount:8,
        access:8,
        offsetHigh:16;  /* offset 16..31 */
} gateDesc_t;

extern gateDesc_t idtTab[TRAPTAB_LENGTH];
extern pseudoDesc_t idtDesc;

trapHandler_t trapHandlersTab[TRAPTAB_LENGTH];
partitionControlTable_t partitionControlTable __attribute__ ((section (".xm_ctrl"), aligned(0x1000)));
partitionInformationTable_t partitionInformationTable __attribute__ ((section (".xm_ctrl"), aligned(0x1000)));

static void UnexpectedTrap(trapCtxt_t *ctxt) {
    printf("[P%d] Unexpected trap 0x%x (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->irqNr, ctxt->eip);
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

void DoTrap(trapCtxt_t *ctxt) {
    if (trapHandlersTab[ctxt->irqNr]) {
        trapHandlersTab[ctxt->irqNr](ctxt);
    }
}

static inline void HwSetIrqGate(xm_s32_t e, void *hndl) {
    idtTab[e].selector=GUEST_CS_SEL;
    idtTab[e].offsetLow=(xmAddress_t)hndl&0xffff;
    idtTab[e].offsetHigh=((xmAddress_t)hndl>>16)&0xffff;
    idtTab[e].access=0x8e|(1<<5);
}

static inline void HwSetTrapGate(xm_s32_t e, void *hndl) {
    idtTab[e].selector=GUEST_CS_SEL;
    idtTab[e].offsetLow=(xmAddress_t)hndl&0xffff;
    idtTab[e].offsetHigh=((xmAddress_t)hndl>>16)&0xffff;
    idtTab[e].access=0x8f|(1<<5);
}

void InitArch(void) {
    extern void (*trapTable[0])(void);
    xm_s32_t irqNr, e;
    
    HwSetTrapGate(0, trapTable[0]);
    HwSetIrqGate(1, trapTable[1]);
    HwSetIrqGate(2, trapTable[2]);
    HwSetTrapGate(3, trapTable[3]);
    HwSetTrapGate(4, trapTable[4]);
    HwSetTrapGate(5, trapTable[5]);
    HwSetTrapGate(6, trapTable[6]);
    HwSetTrapGate(7, trapTable[7]);
    HwSetTrapGate(8, trapTable[8]);
    HwSetTrapGate(9, trapTable[9]);
    HwSetTrapGate(10, trapTable[10]);
    HwSetTrapGate(11, trapTable[11]);
    HwSetTrapGate(12, trapTable[12]);
    HwSetTrapGate(13, trapTable[13]);
    HwSetIrqGate(13, trapTable[13]);
    HwSetIrqGate(14, trapTable[14]);
    HwSetTrapGate(15, trapTable[15]);
    HwSetTrapGate(16, trapTable[16]);
    HwSetTrapGate(17, trapTable[17]);
    HwSetTrapGate(18, trapTable[18]);
    HwSetTrapGate(19, trapTable[19]);

    for (irqNr=0x20; irqNr<TRAPTAB_LENGTH; irqNr++)
        HwSetIrqGate(irqNr, trapTable[irqNr]);

    for (e=0; e<XM_VT_EXT_MAX; e++)
        partitionControlTable.extIrq2Vector[e]=e+EXT_IRQ_VECTOR;

    for (e=0; e<TRAPTAB_LENGTH; e++)
        trapHandlersTab[e]=UnexpectedTrap;

    partitionControlTable.arch.idtr=(pseudoDesc_t)idtDesc;
}
