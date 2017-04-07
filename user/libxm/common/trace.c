/*
 * $FILE: trace.c
 *
 * Tracing functionality
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
#include <xm_inc/objdir.h>
#include <xm_inc/objects/trace.h>

__stdcall xm_s32_t XM_trace_open(xmId_t id) {
    if (!(libXmParams.partInfTab->flags&XM_PART_SUPERVISOR))
        return XM_PERM_ERROR;

    if ((id!=XM_HYPERVISOR_ID)&&((id<0)||(id>=libXmParams.partInfTab->noPartitions)))
        return XM_INVALID_PARAM;
	
    return OBJDESC_BUILD(OBJ_CLASS_TRACE, id, 0);
}

__stdcall xm_s32_t XM_trace_event(xm_u32_t bitmask, xmTraceEvent_t *event) {
    xm_s32_t ret;

    ret=XM_write_object(OBJDESC_BUILD(OBJ_CLASS_TRACE, XM_PARTITION_SELF, 0), event, sizeof(xmTraceEvent_t), &bitmask);
    if (ret < 0)
        return ret;
    if (ret<sizeof(xmTraceEvent_t))
        return XM_INVALID_CONFIG;
    return XM_OK;
}

__stdcall xm_s32_t XM_trace_read(xm_s32_t traceStream, xmTraceEvent_t *traceEventPtr) {
    xm_s32_t ret;
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    /*if (XM_read_object(traceStream, traceEventPtr, sizeof(xmTraceEvent_t), 0)<sizeof(xmTraceEvent_t))*/
	/*return XM_INVALID_CONFIG;*/
    /*return XM_OK;*/
    ret=XM_read_object(traceStream, traceEventPtr, sizeof(xmTraceEvent_t), 0);
    return (ret>0)?(ret/sizeof(xmTraceEvent_t)):ret;
}

__stdcall xm_s32_t XM_trace_seek(xm_s32_t traceStream, xm_s32_t offset, xm_u32_t whence) {
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    return XM_seek_object(traceStream, offset, whence);
}

__stdcall xm_s32_t XM_trace_status(xm_s32_t traceStream, xmTraceStatus_t *traceStatusPtr) {
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    return XM_ctrl_object(traceStream, XM_TRACE_GET_STATUS, traceStatusPtr);
}
