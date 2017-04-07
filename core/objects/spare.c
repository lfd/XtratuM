/*
 * $FILE: spare.c
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
#include <assert.h>
#include <boot.h>
#include <hypercalls.h>
#include <objdir.h>
#include <sched.h>
#include <stdc.h>
#include <objects/spare.h>

static xm_s32_t CtrlSpare(xmObjDesc_t desc, xm_u32_t cmd, union spareCmd *__gParam args) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmId_t partId;

    partId=OBJDESC_GET_PARTITIONID(desc);
    if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SPARE_HOST_F)) {
        return XM_PERM_ERROR;
    }

    if (__CheckGParam(0, args, sizeof(union spareCmd))<0) {
        return XM_INVALID_PARAM;
    }

    switch (cmd) {
    case XM_SPARE_GATHER_DATA:
        if (__CheckGParam(0, args->spareData, sizeof(struct xmcSchedSparePlan))<0) {
            return XM_INVALID_PARAM;
        }
        FillSparePartitionTab(args->spareData);
        return XM_OK;

    case XM_SPARE_SET_PLAN:
        if (!args->spareData)   // Unset spare plan
            return (SetSparePlan(args->spareData)==0)?XM_OK:XM_INVALID_PARAM;

        if (__CheckGParam(0, args->spareData, sizeof(struct xmcSchedSparePlan))<0)
            return XM_INVALID_PARAM;

        return (SetSparePlan(args->spareData)==0)?XM_OK:XM_INVALID_PARAM;

    case XM_SPARE_GET_PLAN:
        if (__CheckGParam(0, args->spareData, sizeof(struct xmcSchedSparePlan))<0)
            return XM_INVALID_PARAM;

        memcpy(args->spareData,
               GetCurrentSparePlan(),
               sizeof(struct xmcSchedSparePlanHeader) + GetCurrentSparePlan()->header.noSlots*sizeof(struct xmcSchedSpareSlot));
        return XM_OK;
    }
    return XM_INVALID_PARAM;
}

static const struct object spareObj={
    .Ctrl=(ctrlObjOp_t)CtrlSpare,
};

xm_s32_t __VBOOT SetupSpare(void) {
    objectTab[OBJ_CLASS_SPARE]=&spareObj;
    return 0;
}

REGISTER_OBJ(SetupSpare);
