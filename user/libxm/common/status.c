/*
 * $FILE: status.c
 *
 * Status functionality
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <status.h>
#include <xm_inc/objects/status.h>

__stdcall xm_s32_t XM_system_get_status(xmSystemStatus_t *status) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_HYPERVISOR_ID, 0), XM_STATUS_GET, status);
}

__stdcall xm_s32_t XM_partition_get_status(xmId_t id, xmPartitionStatus_t *status) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, id, 0), XM_STATUS_GET, status);
}

__stdcall xm_s32_t XM_set_partition_opmode(xm_s32_t opMode) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_PARTITION_SELF, 0), XM_SET_PARTITION_OPMODE, &opMode);
}

__stdcall xm_s32_t XM_set_plan(xm_u32_t planId) {
    union statusCmd cmd;
    cmd.schedPlan.new=planId;
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_HYPERVISOR_ID, 0), XM_SWITCH_SCHED_PLAN, &cmd);
}

__stdcall xm_s32_t XM_get_plan_status(xmPlanStatus_t *status) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_PARTITION_SELF, 0), XM_GET_SCHED_PLAN_STATUS, status);
}
