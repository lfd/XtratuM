/*
 * $FILE: commports.h
 *
 * Communication port object definitions
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJ_COMMPORTS_H_
#define _XM_OBJ_COMMPORTS_H_

// Commands
#define XM_COMM_CREATE_PORT 0x0
#define XM_COMM_GET_PORT_STATUS 0x1

#ifndef __gParam
#define __gParam
#endif

/* <track id="sampling-status-type"> */
typedef struct {
    xmTime_t  validPeriod; // Refresh period.
    xm_u32_t  maxMsgSize;  // Max message size.
    xm_u32_t  flags;
    xmTime_t  timestamp;
    xm_u32_t  lastMsgSize;
} xmSamplingPortStatus_t;
/* </track id="sampling-status-type"> */

//@ \void{<track id="sampling-info-type">}
typedef struct {
    char *__gParam portName;
    xmTime_t  refreshPeriod; // Refresh period.
    xm_u32_t  maxMsgSize;  // Max message size.
    xm_u32_t direction;
} xmSamplingPortInfo_t;
//@ \void{</track id="sampling-info-type">}

/* <track id="queuing-status-type"> */
typedef struct {
    xmTime_t  validPeriod; // Refresh period.
    xm_u32_t  maxMsgSize;  // Max message size.
    xm_u32_t  maxNoMsgs;      // Max number of messages.
    xm_u32_t  noMsgs;       // Current number of messages.
    xm_u32_t  flags;
} xmQueuingPortStatus_t;
/* </track id="queuing-status-type"> */

//@ \void{<track id="queuing-info-type">}
typedef struct {
    char *__gParam portName;
    xm_u32_t maxMsgSize; // Max message size.
    xm_u32_t maxNoMsgs; // Max number of messages.
    xm_u32_t direction;
} xmQueuingPortInfo_t;
//@ \void{</track id="queuing-info-type">}

union samplingPortCmd {
    struct createSCmd {
	char *__gParam portName; 
	xm_u32_t maxMsgSize; 
	xm_u32_t direction;
    } create;
    xmSamplingPortStatus_t status;
};

union queuingPortCmd {
    struct createQCmd {
	char *__gParam portName; 
	xm_u32_t maxNoMsgs;
	xm_u32_t maxMsgSize;
	xm_u32_t direction;
    } create;
    xmQueuingPortStatus_t status;
};

#define XM_COMM_MSG_VALID 0x1

#endif
