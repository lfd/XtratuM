/*
 * $FILE: hypercalls.c
 *
 * Lib XM Linux (user version)
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include "xm-linux.h"

// Time management hypercalls
__stdcall xm_s32_t XM_get_time(xm_u32_t clock_id, xmTime_t * time) {
    return XM_hcall(get_time_nr, clock_id, (xm_u32_t) time, 0, 0, 0);
}

__stdcall xm_s32_t XM_set_timer(xm_u32_t clock_id, xmTime_t abstime, xmTime_t interval) {
    return XM_hcall(set_timer_nr, clock_id, (xm_u32_t) abstime, (xm_u32_t) interval, 0, 0);
}

// Partition status hypercalls
__stdcall xm_s32_t XM_suspend_partition(xm_u32_t partition_id) {
    return XM_hcall(suspend_partition_nr, partition_id, 0, 0, 0, 0);
}

__stdcall xm_s32_t XM_resume_partition(xm_u32_t partition_id) {
    return XM_hcall(resume_partition_nr, partition_id, 0, 0, 0, 0);
}

__stdcall xm_s32_t XM_shutdown_partition(xm_u32_t partition_id) {
    return XM_hcall(shutdown_partition_nr, partition_id, 0, 0, 0, 0);
}

__stdcall xm_s32_t XM_reset_partition(xm_u32_t partition_id, xm_u32_t resetMode, xm_u32_t status) {
    return XM_hcall(reset_partition_nr, partition_id, resetMode, status, 0, 0);
}

__stdcall xm_s32_t XM_halt_partition(xm_u32_t partition_id) {
    return XM_hcall(halt_partition_nr, partition_id, 0, 0, 0, 0);
}

__stdcall xm_s32_t XM_idle_self(void) {
    return XM_hcall(idle_self_nr, 0, 0, 0, 0, 0);
}

// system status hypercalls
__stdcall xm_s32_t XM_halt_system(void) {
    return XM_hcall(halt_system_nr, 0, 0, 0, 0, 0);
}

__stdcall xm_s32_t XM_reset_system(xm_u32_t resetMode) {
    return XM_hcall(reset_system_nr, resetMode, 0, 0, 0, 0);
}

// Object related hypercalls
__stdcall xm_s32_t XM_read_object(xmObjDesc_t objDesc, void *buffer, xm_u32_t size, xm_u32_t * flags) {
    return XM_hcall(read_object_nr, objDesc, (xm_u32_t) buffer, size, (xm_u32_t) flags, 0);
}

__stdcall xm_s32_t XM_write_object(xmObjDesc_t objDesc, void *buffer, xm_u32_t size, xm_u32_t * flags) {
    return XM_hcall(write_object_nr, objDesc, (xm_u32_t) buffer, size, (xm_u32_t) flags, 0);
}

__stdcall xm_s32_t XM_seek_object(xmObjDesc_t objDesc, xm_u32_t offset, xm_u32_t whence) {
    return XM_hcall(seek_object_nr, objDesc, offset, whence, 0, 0);
}

__stdcall xm_s32_t XM_ctrl_object(xmObjDesc_t objDesc, xm_u32_t cmd, void *arg) {
    return XM_hcall(ctrl_object_nr, objDesc, cmd, (xm_u32_t) arg, 0, 0);
}
