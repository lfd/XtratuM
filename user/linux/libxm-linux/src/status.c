/*
 * $FILE: status.c
 *
 * Status functionality
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include "xm-linux.h"

__stdcall xm_s32_t XM_system_get_status(xmSystemStatus_t * status) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_HYPERVISOR_ID, 0), XM_STATUS_GET, status);
}

__stdcall xm_s32_t XM_partition_get_status(xmId_t id, xmPartitionStatus_t * status) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, id, 0), XM_STATUS_GET, status);
}

__stdcall xm_s32_t XM_set_plan(xm_u32_t planId) {
    union statusCmd cmd;
    cmd.schedPlan.new=planId;
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_HYPERVISOR_ID, 0), XM_SWITCH_SCHED_PLAN, &cmd);
}

__stdcall xm_s32_t XM_get_plan_status(xmPlanStatus_t *status) {
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_STATUS, XM_PARTITION_SELF, 0), XM_GET_SCHED_PLAN_STATUS, status);
}
