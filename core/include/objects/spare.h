/*
 * $FILE: spare.h
 *
 * Spare Plan Manager
 *
 * $VERSION$
 *
 * Author: Jordi Sánchez, <jsanchez@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */
#ifndef _XM_OBJ_SPARE_H_
#define _XM_OBJ_SPARE_H_

#ifndef __gParam
#define __gParam
#endif

#define XM_SPARE_GATHER_DATA 0x0
#define XM_SPARE_SET_PLAN 0x1
#define XM_SPARE_GET_PLAN 0x2
#define XM_SPARE_LAUNCH_PLAN 0x3

#define XM_PCT_SLOT_CYCLIC 0x0
#define XM_PCT_SLOT_SPARE 0x1
#define XM_PCT_SLOT_SPLIT_P1 0x10
#define XM_PCT_SLOT_SPLIT_P2 0x20
#define XM_PCT_SLOT_SPLIT 0x30

struct xmcSchedSpareSlot {
    xm_u32_t id;
    xmId_t partitionId;
    xm_u32_t duration;
    //xm_u32_t sExec; // offset (usec)
    //xm_u32_t eExec; // offset+duration (usec)
};

struct xmcSchedSparePlanHeader {
    xm_u32_t noSlots;
    xmTime_t cycleTime;
};

struct xmcSchedSparePlan {
    struct xmcSchedSparePlanHeader header;
    struct xmcSchedSpareSlot *slotTab;
};

/*
typedef struct {

} xmSpareData_t;
*/

union spareCmd {
    struct xmcSchedSparePlan *__gParam spareData;
};

#endif //_XM_OBJ_SPARE_H_
