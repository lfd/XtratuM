/*
 * $FILE: trace.h
 *
 * tracing subsystem
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _LIB_XM_STATUS_H_
#define _LIB_XM_STATUS_H_

#include <xm_inc/config.h>
#include <xm_inc/objdir.h>
#include <xm_inc/objects/status.h>

extern __stdcall xm_s32_t XM_partition_get_status(xmId_t id, xmPartitionStatus_t *status);
extern __stdcall xm_s32_t XM_set_partition_opmode(xm_s32_t opMode);
extern __stdcall xm_s32_t XM_system_get_status (xmSystemStatus_t  *status);
extern __stdcall xm_s32_t XM_set_plan(xm_u32_t planId);
extern __stdcall xm_s32_t XM_get_plan_status(xmPlanStatus_t *status);

#endif
