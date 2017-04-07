/*
 * $FILE: hm.c
 *
 * Health Monitor
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
#include <hm.h>
#include <xm_inc/objects/hm.h>

__stdcall xm_s32_t XM_hm_open(void) {
    return XM_OK;
}

//@ \void{<\track id="using-proc">}
__stdcall xm_s32_t XM_hm_read(xmHmLog_t *hmLogPtr) {
    if (!hmLogPtr)
	return XM_INVALID_PARAM;
    return (XM_read_object(OBJDESC_BUILD(OBJ_CLASS_HM, XM_HYPERVISOR_ID, 0), hmLogPtr, sizeof(xmHmLog_t), 0)<sizeof(xmHmLog_t))?XM_INVALID_CONFIG:XM_OK;
}

__stdcall xm_s32_t XM_hm_seek(xm_s32_t offset, xm_u32_t whence) {
    return XM_seek_object(OBJDESC_BUILD(OBJ_CLASS_HM, XM_HYPERVISOR_ID, 0), offset, whence);
}
 
__stdcall xm_s32_t XM_hm_status(xmHmStatus_t *hmStatusPtr) {
    if (!hmStatusPtr)
	return XM_INVALID_PARAM;
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_HM, XM_HYPERVISOR_ID, 0), XM_HM_GET_STATUS, hmStatusPtr);
}

