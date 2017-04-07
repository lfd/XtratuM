/*
 * $FILE: trace.c
 *
 * XM Partition Manager: trace support
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include "common.h"

static int CmdTraceOpen(int traceid){
        xm_s32_t ret;

        ret=XM_trace_open(traceid);
        if(ret<0){
            xmprintf("Error: trace open %d failed with %d: %s\n", traceid, ret, ErrorToStr(ret));
            return ret;
        }
        return ret;
}

// TODO stateless trace operation
int CmdTrace(char *line){
    xm_s32_t ret;
    int narg;
    char *arg[2];
    char *operlist[] = {"event", "open", "read", "seek", "status"};

    xm_s32_t traceid, traceStream, offset=0, whence=0;
    xm_u32_t bitmask=0;
    xmTraceStatus_t traceStatus;
    xmTraceEvent_t traceEvent;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: operation not provided\n");
        return -1;
    }

    if(narg == 0){
        xmprintf("Error: traceid not provided\n");
        return -1;
    }

    traceid = atoi(arg[1]);
    switch(GetCommandIndex(arg[0], operlist, NELEM(operlist))){
    default:
        ret=-1;
        break;

    case 0:
        traceStream=CmdTraceOpen(traceid);
        ret=XM_trace_event(bitmask, &traceEvent);
        break;
    case 1:
        traceStream=CmdTraceOpen(traceid);
        break;
    case 2:
        traceStream=CmdTraceOpen(traceid);
        ret=XM_trace_read(traceStream, &traceEvent);
        if(ret<0){
            xmprintf("Error: trace read %d failed with %d: %s\n", traceStream, ret, ErrorToStr(ret));
            break;
        }

        xmprintf("trace read stream %d\n"
                "opcode 0x%x reserved 0x%x timestamp %lld\n"
                "word 0x%x str: %s\n",
                traceEvent.opCode, traceEvent.reserved, traceEvent.timeStamp,
                traceEvent.word, traceEvent.str);
        break;
    case 3:
        traceStream=CmdTraceOpen(traceid);
        ret=XM_trace_seek(traceStream, offset, whence);
        break;
    case 4:
        traceStream=CmdTraceOpen(traceid);
        ret=XM_trace_status(traceStream, &traceStatus);
        xmprintf("trace status stream: %d\n"
                 "noEvents: %ld maxEvents: %d currentEvent: %ld\n",
                traceStream, traceStatus.noEvents, traceStatus.maxEvents, traceStatus.currentEvent);
        break;
    }
    
    return ret;
}

