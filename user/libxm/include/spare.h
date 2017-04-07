/*
 * $FILE: spare.h
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
#ifndef _LIB_XM_SPARE_H_
#define _LIB_XM_SPARE_H_

#include <xm_inc/config.h>
#include <xm_inc/objdir.h>
#include <xm_inc/objects/spare.h>

#include <xm_inc/linkage.h>

extern __stdcall xm_s32_t XM_gather_spare_data(struct xmcSchedSparePlan *spareData);
extern __stdcall xm_s32_t XM_set_spare_plan(struct xmcSchedSparePlan *spareData);
extern __stdcall xm_s32_t XM_get_spare_plan(struct xmcSchedSparePlan *spareData);

#endif //_LIB_XM_SPARE_H_
