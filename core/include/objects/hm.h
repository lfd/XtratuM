/*
 * $FILE: hm.h
 *
 * Health Monitor definitions
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJ_HM_H_
#define _XM_OBJ_HM_H_

struct hmCpuCtxt {
    xm_u32_t ip;
    xm_u32_t flags;
    xm_s32_t irqNr;
};

#define CpuCtxt2HmCpuCtxt(cpuCtxt, hmCpuCtxt) do { \
    (hmCpuCtxt)->ip=(cpuCtxt)->eip; \
    (hmCpuCtxt)->flags=(cpuCtxt)->eflags; \
    (hmCpuCtxt)->irqNr=(cpuCtxt)->irqNr; \
} while(0)

/* <track id="xm-hm-log-msg"> */
typedef struct {
    xm_u32_t eventId:13, system:1, reserved:2, moduleId:8, partitionId:8;
    union {
#define XM_HMLOG_PAYLOAD_LENGTH 5
        struct hmCpuCtxt cpuCtxt;
        xm_u32_t word[XM_HMLOG_PAYLOAD_LENGTH];
    };
    xmTime_t timeStamp;
} xmHmLog_t;
/* </track id="xm-hm-log-msg"> */

#define XM_HM_GET_STATUS 0x0

/* <track id="xm-hm-log-status"> */
typedef struct {
    xm_s32_t noEvents;
    xm_s32_t maxEvents;
    xm_s32_t currentEvent;
} xmHmStatus_t;
/* </track id="xm-hm-log-status"> */

union hmCmd {
    xmHmStatus_t status;
};

#ifdef _XM_KERNEL_
extern xm_s32_t HmRaiseEvent(xmHmLog_t *log);
#endif
#endif
