/*
 * $FILE: spare.c
 *
 * Spare plan manager
 *
 * $VERSION$
 *
 * Author: Jordi Sánchez, <jsanchez@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */
#include <xm.h>
#include <spare.h>
#include <xm_inc/objects/spare.h>

__stdcall xm_s32_t XM_gather_spare_data(struct xmcSchedSparePlan *spareData) {
    union spareCmd spareCmd;
    spareCmd.spareData = spareData;
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_SPARE, XM_HYPERVISOR_ID, 0), XM_SPARE_GATHER_DATA, &spareCmd);
}

__stdcall xm_s32_t XM_set_spare_plan(struct xmcSchedSparePlan *spareData) {
    union spareCmd spareCmd;
    spareCmd.spareData = spareData;
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_SPARE, XM_HYPERVISOR_ID, 0), XM_SPARE_SET_PLAN, &spareCmd);
}

__stdcall xm_s32_t XM_get_spare_plan(struct xmcSchedSparePlan *spareData) {
    union spareCmd spareCmd;
    spareCmd.spareData = spareData;
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_SPARE, XM_HYPERVISOR_ID, 0), XM_SPARE_GET_PLAN, &spareCmd);
}
