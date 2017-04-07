/*
 * $FILE: trace.c
 *
 * Tracing functionality
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

#include <xm_inc/objdir.h>
#include <xm_inc/objects/trace.h>

__stdcall xm_s32_t XM_trace_open(xmId_t id) {

    if (id!=XM_PARTITION_SELF) {
	// miss: Check whether this is a SV partition
	//if ()
	//return XM_PERM_ERROR;
	
    }
	
    return OBJDESC_BUILD(OBJ_CLASS_CONSOLE, id, 0);
}

__stdcall xm_s32_t XM_trace_event(xm_u32_t bitmask, xmTraceEvent_t *event) {
    if (!event)
	return XM_INVALID_PARAM;

    if (XM_write_object(OBJDESC_BUILD(OBJ_CLASS_CONSOLE, XM_PARTITION_SELF, 0), event, sizeof(xmTraceEvent_t), &bitmask)<sizeof(xmTraceEvent_t))
	return XM_INVALID_CONFIG;
    return XM_OK;
}

__stdcall xm_s32_t XM_trace_read(xm_s32_t traceStream, xmTraceEvent_t *traceEventPtr) {
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    if (!traceEventPtr)
	return XM_INVALID_PARAM;

    if (XM_read_object(traceStream, traceEventPtr, sizeof(xmTraceEvent_t), 0)<sizeof(xmTraceEvent_t))
	return XM_INVALID_CONFIG;
    return XM_OK;
}

__stdcall xm_s32_t XM_trace_seek(xm_s32_t traceStream, xm_s32_t offset, xm_u32_t whence) {
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    return XM_seek_object(traceStream, offset, whence);
}

__stdcall xm_s32_t XM_trace_status(xm_s32_t traceStream, xmTraceStatus_t *traceStatusPtr) {
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    if (!traceStatusPtr)
	return XM_INVALID_PARAM;
    return XM_ctrl_object(traceStream, XM_TRACE_GET_STATUS, traceStatusPtr);
}
