/*
 * $FILE: status.h
 *
 * System/partition status
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJ_STATUS_H_
#define _XM_OBJ_STATUS_H_

/* <track id="system-status"> */
typedef struct {
    xm_u32_t resetCounter;
    /* Number of HM events emmite. */
    xm_u64_t noHmEvents;                /* [[OPTIONAL]] */
    /* Number of HW interrupts received. */
    xm_u64_t noIrqs;                    /* [[OPTIONAL]] */
    /* Current major cycle interation. */
    xm_u64_t currentMaf;                /* [[OPTIONAL]] */
    /* Total number of system messages: */
    xm_u64_t noSamplingPortMsgsRead;    /* [[OPTIONAL]] */
    xm_u64_t noSamplingPortMsgsWritten; /* [[OPTIONAL]] */
    xm_u64_t noQueuingPortMsgsSent;     /* [[OPTIONAL]] */
    xm_u64_t noQueuingPortMsgsReceived; /* [[OPTIONAL]] */
} xmSystemStatus_t;
/* </track id="system-status"> */

//@ \void{<track id="plan-status">}
typedef struct {
    xmTime_t switchTime;
    xm_s32_t next;
    xm_s32_t current;
    xm_s32_t prev;
} xmPlanStatus_t;
//@ \void{</track id="plan-status">} 

/* <track id="partition-status"> */
typedef struct {
    /* Current state of the partition: ready, suspended ... */
    xm_u32_t state;
#define XM_STATUS_IDLE 0x0
#define XM_STATUS_READY 0x1
#define XM_STATUS_SUSPENDED 0x2
#define XM_STATUS_HALTED 0x3
    /* Number of virtual interrupts received. */
    xm_u64_t noVIrqs;                   /* [[OPTIONAL]] */
    /* Reset information */
    xm_u32_t resetCounter;
    xm_u32_t resetStatus;
    xmTime_t execClock;
    /* Total number of partition messages: */
    xm_u64_t noSamplingPortMsgsRead;    /* [[OPTIONAL]] */
    xm_u64_t noSamplingPortMsgsWritten; /* [[OPTIONAL]] */
    xm_u64_t noQueuingPortMsgsSent;     /* [[OPTIONAL]] */
    xm_u64_t noQueuingPortMsgsReceived; /* [[OPTIONAL]] */   
} xmPartitionStatus_t;
/* </track id="partition-status"> */

#define  XM_STATUS_GET 0x0
#define  XM_GET_SCHED_PLAN_STATUS 0x1
#define  XM_SWITCH_SCHED_PLAN 0x2

union statusCmd {
    union {
	xmSystemStatus_t system;
	xmPartitionStatus_t partition;
        xmPlanStatus_t plan;
    } status;
    struct {
        xm_u32_t new;
        xm_u32_t current;
    } schedPlan;
};

#ifdef _XM_KERNEL_
extern xmSystemStatus_t systemStatus;
extern xmPartitionStatus_t *partitionStatus;
#endif
#endif
