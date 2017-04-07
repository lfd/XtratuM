/*
 * $FILE: spare.h
 *
 * Spare scheduling library
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */
#ifndef _SPARE_H_
#define _SPARE_H_

#define SPARE_MEM_FLAG  XM_MEM_AREA_FLAG1
#define SPARE_MAGIC     0xA55A5AA5
#define PARTITION_BUSY  1

enum {
    SCHED_CLASS_PRIORITY=0,
    SCHED_CLASS_BANDWIDTH,
    SCHED_CLASS_IDLE,
    SCHED_CLASS_MAX,
};

struct SchedData {
    xm_u32_t policy;
    union {
        struct {
            xmTime_t budget;
            xmTime_t run;
            xm_u32_t ratio;
        } bw;
        xm_u32_t priority;
    };
};

struct SpareHeader {
    xm_u32_t magic;
    xm_u32_t partitionId;
    xm_u32_t busy;
};

#endif /*_SPARE_H_*/
