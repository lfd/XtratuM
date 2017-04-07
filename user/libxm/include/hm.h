/*
 * $FILE: hm.h
 *
 * health monitor
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _LIB_XM_HM_H_
#define _LIB_XM_HM_H_

#include <xm_inc/config.h>
#include <xm_inc/objdir.h>
#include <xm_inc/objects/hm.h>

extern __stdcall xm_s32_t XM_hm_open(void) __attribute__((__deprecated__));
extern __stdcall xm_s32_t XM_hm_read(xmHmLog_t *hmLogPtr);
extern __stdcall xm_s32_t XM_hm_seek(xm_s32_t offset, xm_u32_t whence);
extern __stdcall xm_s32_t XM_hm_status(xmHmStatus_t *hmStatusPtr);

#endif
