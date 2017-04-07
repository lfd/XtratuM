/*
 * $FILE: gaccess.h
 *
 * Guest shared info
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_GACCESS_H_
#define _XM_ARCH_GACCESS_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <arch/xm_def.h>

#define __archGParam
#define __ArchCheckGParam(__ctxt, __param, __size) \
({ \
        xm_s32_t __r=0;\
        localSched_t *__sched=GET_LOCAL_SCHED();\
        const struct xmcPartition *__cfg=__sched->cKThread->ctrl.g->cfg; \
        xm_s32_t __e; \
        if (((xmAddress_t)__param+__size)>=CONFIG_XM_OFFSET) __r=-1; \
        if (1 && __param && (__sched->cKThread->ctrl.g->partitionControlTable->arch.cr3 > __sched->cKThread->ctrl.g->partitionControlTable->arch.atomicArea.eAddr)) { \
            for (__e=0; __e<__cfg->noPhysicalMemoryAreas; __e++) {\
                xm_u32_t sAddr=xmcPhysMemAreaTab[__e+__sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset].startAddr; \
                xm_u32_t eAddr=sAddr+xmcPhysMemAreaTab[__e+__sched->cKThread->ctrl.g->cfg->physicalMemoryAreasOffset].size; \
                if ((((xm_u32_t)__param)>=sAddr)&&(((xm_u32_t)__param+(xm_u32_t)__size)<eAddr)) \
                    break; \
            } \
            if (__e>=__cfg->noPhysicalMemoryAreas) \
                __r=-1; \
        }\
        __r; \
})

#endif
