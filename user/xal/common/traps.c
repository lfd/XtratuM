/*
 * $FILE: traps.c
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

/* <track id="xm-traps-c"> */

#include <xm.h>
#include <stdio.h>
#include <irqs.h>

partitionControlTable_t partitionControlTable __attribute__ ((section (".xm_ctrl"), aligned(0x1000)));
partitionInformationTable_t partitionInformationTable __attribute__ ((section (".xm_ctrl"), aligned(0x1000)));

void __attribute__ ((weak)) ExceptionHandler(xm_s32_t trapNr) {
    printf("exception 0x%x (%d)\n", trapNr, trapNr);
    XM_halt_partition(XM_PARTITION_SELF);
}

void __attribute__ ((weak)) ExtIrqHandler(xm_s32_t trapNr) {
    printf("extIrq 0x%x (%d)\n", trapNr, trapNr);
    XM_halt_partition(XM_PARTITION_SELF);
}

void __attribute__ ((weak)) HwIrqHandler(xm_s32_t trapNr) {
    printf("hwIrq 0x%x (%d)\n", trapNr, trapNr);
    XM_halt_partition(XM_PARTITION_SELF);
}

/* </track id="xm-traps-c"> */
