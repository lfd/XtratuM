/*
 * $FILE: trace.h
 *
 * Tracing object definitions
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJ_TRACE_H_
#define _XM_OBJ_TRACE_H_

/* <track id="xm-trace-opcode"> */
typedef struct {
    xm_u32_t code:13, criticality:3, moduleId:8, partitionId:8;
#define XM_TRACE_UNRECOVERABLE 0x3 // This level triggers a health
				   // monitoring fault
#define XM_TRACE_WARNING 0x2
#define XM_TRACE_DEBUG 0x1
#define XM_TRACE_NOTIFY 0x0
} xmTraceOpCode_t;
/* </track id="xm-trace-opcode"> */

/* <track id="xm-trace-event"> */
typedef struct {
    xmTraceOpCode_t opCode;
    xm_u32_t reserved;
    xmTime_t timeStamp;
    union {
        xm_u32_t word[4];
        char str[16];
    };
} xmTraceEvent_t;
/* </track id="xm-trace-event"> */

#define XM_TRACE_GET_STATUS 0x0

/* <track id="xm-trace-status"> */
typedef struct {
    xm_s32_t noEvents;
    xm_s32_t maxEvents;
    xm_s32_t currentEvent;
} xmTraceStatus_t;
/* </track id="xm-trace-status"> */

//@ \void{<track id="xm-trace-bitmap">}

//@ \void{</track id="xm-trace-bitmap">}

union traceCmd {
    xmTraceStatus_t status;
};

#ifdef _XM_KERNEL_
extern xm_s32_t TraceWriteSysEvent(xm_u32_t bitmap, xmTraceEvent_t *event);
#endif
#endif
