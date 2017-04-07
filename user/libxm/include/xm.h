/*
 * $FILE: xm.h
 *
 * Guest header file
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _LIB_XM_H_
#define _LIB_XM_H_

#ifdef _XM_KERNEL_
#error Guest file, do not include.
#endif

#include <xm_inc/config.h>
#include <xm_inc/xmef.h>
#include <xmhypercalls.h>

#ifndef __ASSEMBLY__

#include <xm_inc/hypercalls.h>
#include <xm_inc/guest.h>

extern struct libXmParams {
    partitionControlTable_t *partCtrlTab;
    partitionInformationTable_t *partInfTab;
} libXmParams;

extern __stdcall void init_libxm(partitionControlTable_t *partCtrlTab, partitionInformationTable_t *partInfTab);

static inline partitionInformationTable_t *XM_params_get_PIT(void) {
    return libXmParams.partInfTab;
}

static inline partitionControlTable_t *XM_params_get_PCT(void) {
    return libXmParams.partCtrlTab;
}

#include <comm.h>
#include <hm.h>
#include <hypervisor.h>
#include <trace.h>
#include <status.h>
#ifdef CONFIG_SPARE_SCHEDULING
#include <spare.h>
#endif

#endif

#endif
