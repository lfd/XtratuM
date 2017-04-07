/*
 * $FILE: trace.c
 *
 * Trace event generator Partition
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

/* <track id="hm-monitor-trace"> */
#include <string.h>
#include <stdio.h>
#include <xm.h>
#include "error.h"
#define NR_TRACES 3

xmTraceEvent_t event;
void PartitionMain(void) {
    xm_s32_t e, ret;
    xm_u32_t bitmask = 0x3;

    printf("[P%d] HM Trace Partition start\n", XM_PARTITION_SELF);
    for (e=0; e < NR_TRACES; e++) {
        event.opCode.moduleId=e;
        event.opCode.criticality=2;
        event.word[0]=0x00112233;
        event.word[1]=0x44556677;
        event.word[2]=0x8899AABB;
        event.word[3]=0xCCDDEEFF;
        ret=XM_trace_event(bitmask, &event);
        if(ret != XM_OK)
            eprintf("[P%d] HM Trace event (%x, %d): FAILED %d\n", XM_PARTITION_SELF, bitmask, e, ret);
        printf("[P%d] HM Trace event (%x, %d)\n", XM_PARTITION_SELF, bitmask, e);
        XM_idle_self();
    }
    XM_halt_partition(XM_PARTITION_SELF);
}
/* </track id="hm-monitor-trace"> */
