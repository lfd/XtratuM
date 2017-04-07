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

#ifndef _LIB_XM_TRACE_H_
#define _LIB_XM_TRACE_H_

#include <xm_inc/config.h>
#include <xm_inc/objdir.h>
#include <xm_inc/objects/trace.h>

extern __stdcall xm_s32_t XM_trace_event(xm_u32_t bitmask, xmTraceEvent_t *event);
extern __stdcall xm_s32_t XM_trace_open(xmId_t id);
extern __stdcall xm_s32_t XM_trace_read(xm_s32_t traceStream, xmTraceEvent_t *traceEventPtr);
extern __stdcall xm_s32_t XM_trace_seek(xm_s32_t traceStream, xm_s32_t offset, xm_u32_t whence);
#define XM_TRACE_SEEK_CUR XM_OBJ_SEEK_CUR
#define XM_TRACE_SEEK_END XM_OBJ_SEEK_END
#define XM_TRACE_SEEK_SET XM_OBJ_SEEK_SET
extern __stdcall xm_s32_t XM_trace_status(xm_s32_t traceStream, xmTraceStatus_t *traceStatusPtr);

#endif
