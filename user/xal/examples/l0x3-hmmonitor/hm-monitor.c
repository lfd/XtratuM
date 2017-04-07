/*
 * $FILE: hm-monitor.c
 *
 * Health Monitoring Partition
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

/* <track id="hm-monitor-code"> */
#include <stdio.h>
#include <xm.h>
#include <string.h>
#include <stdio.h>
#include "error.h"

static inline void PrintSysStatus(xmSystemStatus_t *s) {
    printf("\n>> System status\n");
    printf("noHmEvents: %d noIrqs: %d currentMaf: %d\n"
           "noSamplingPortMsgsRead: %d noSamplingPortMsgsWritten: %d\n"
           "noQueuingPortMsgsSent: %d noQueuingPortMsgsReceived: %d\n", 
           s->noHmEvents, s->noIrqs, s->currentMaf,
           s->noSamplingPortMsgsRead, s->noSamplingPortMsgsWritten,
           s->noQueuingPortMsgsSent, s->noQueuingPortMsgsReceived);
}
static inline void PrintPartStatus(xm_s32_t partId, xmPartitionStatus_t *p) {
    printf("\n>> Partition status P%d\n", partId);
    printf("execClock: %lld state: %d virqs: %d resetCounter: %d resetStatus: %d\n"
           "noSamplingPortMsgsRead: %d noSamplingPortMsgsWritten: %d\n"
           "noQueuingPortMsgsSent: %d noQueuingPortMsgsReceived: %d\n",
           p->execClock, p->state, p->noVIrqs, p->resetCounter, p->resetStatus,
           p->noSamplingPortMsgsRead, p->noSamplingPortMsgsWritten,
           p->noQueuingPortMsgsSent, p->noQueuingPortMsgsReceived);
}
static void PrintHmLog(xmHmLog_t *h) {
    printf("\n>> Partition HM Event\n");
    printf("partitionId: %x eventId: %x timeStamp: %lld\n", h->partitionId, h->eventId, h->timeStamp);
}
static void PrintTraceEvent(xm_s32_t partId, xmTraceEvent_t *e) {
    xmTraceOpCode_t *o = &e->opCode;
    printf("\n>> Partition Trace Event P%d\n", partId);
    printf("partitionId: %x moduleId: %x criticality: %x code: %x timeStamp: %lld"
        " str[16]: %08x %08x %08x %08x\n",
        o->partitionId, o->moduleId, o->criticality, o->code, e->timeStamp,
        e->word[0], e->word[1], e->word[2], e->word[3]);
}
int GetNoPartitions(void){
    int i;
    xmPartitionStatus_t p;
    for(i=0; XM_partition_get_status(i, &p) == XM_OK; i++) {}
    return i;
}

xmTraceEvent_t event;
xmHmLog_t hmLog;
xmHmStatus_t hmStatus;
xmSystemStatus_t sysStatus;

void PartitionMain(void) {
    xm_s32_t noPartitions = GetNoPartitions();
    xm_s32_t i, ret;
    xm_s32_t xmtsfd, tsfd[noPartitions];
    xmPartitionStatus_t partStatus[noPartitions];
   
    memset(&partStatus, 0, sizeof(partStatus));
    printf("[P%d] HM Monitor Partition start\n", XM_PARTITION_SELF);

    xmtsfd=XM_trace_open(XM_HYPERVISOR_ID);
    ret=XM_trace_read(xmtsfd, &event);
    printf("[P%d] HM Monitor Hypervisor Trace %d\n", xmtsfd, ret);
    
    for(i=0; i < noPartitions; i++){
        tsfd[i]=XM_trace_open(i);
        if(tsfd[i]<0)
            eprintf("[P%d] HM Monitor Trace open %d: FAILED %d\n", XM_PARTITION_SELF, i, tsfd[i]);
        ret=XM_trace_seek(tsfd[i], 0, XM_TRACE_SEEK_SET);
    }
    while(1){
        XM_system_get_status(&sysStatus);
        ret=XM_hm_status(&hmStatus);
        if (ret != XM_OK)
            eprintf("[P%d] read HM status: FAILED %d\n", XM_PARTITION_SELF, ret);
        /* read the hm status pending hm events */
        if(hmStatus.currentEvent < hmStatus.noEvents){
            ret=XM_hm_read(&hmLog);
            if (ret != XM_OK)
                eprintf("[P%d] read hm log: FAILED %d\n", XM_PARTITION_SELF, ret);
        }
        for (i=0; i < noPartitions; i++){
            /* read the state and traces of each partition */
            XM_partition_get_status(i, &partStatus[i]);
            ret=XM_trace_read(tsfd[i], &event);
            /* print the partition status, trace and the hm events */
            PrintPartStatus(i, &partStatus[i]);
            PrintTraceEvent(i, &event);
        }
        PrintSysStatus(&sysStatus);
        PrintHmLog(&hmLog);
        printf("---------------------------------------------------------------\n");
        XM_idle_self();
    }
    XM_halt_partition(XM_PARTITION_SELF);
}
/* </track id="hm-monitor-code"> */
