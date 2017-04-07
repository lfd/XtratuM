/*
 * $FILE: hm.c
 *
 * XM Partition Manager: HM support
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

int CmdHM(char *line){
    xm_s32_t ret;
    int narg;
    char *arg[2];
    char *operlist[] = { "read", "seek", "status"};

    xmHmLog_t hmLog;
    xmHmStatus_t hmStatus;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: operation not provided\n");
        return -1;
    }
    
    if(narg == -1){
        xmprintf("Error: operation not provided\n");
        return -1;
    }

    switch(GetCommandIndex(arg[0], operlist, NELEM(operlist))){
    default:
        ret=-1;
        break;

    case 0:
        ret=XM_hm_read(&hmLog);
        xmprintf("hm read: partitionId: %x eventId: %x timeStamp: %lld\n",
                hmLog.partitionId, hmLog.eventId, hmLog.timeStamp);
        break;
    case 1:
        ret=XM_hm_seek(0, 0);
        xmprintf("hm seek: %d\n", ret);
        break;
    case 2:
        ret=XM_hm_status(&hmStatus);
        xmprintf("hm status: noEvents: %d maxEvents: %d currentEvent: %d\n",
            hmStatus.noEvents, hmStatus.maxEvents, hmStatus.currentEvent);
        break;
    }
    
    return ret;
}
