/*
 * $FILE: brk.h
 *
 * Memory for structure initialisation
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_BRK_H_
#define _XM_BRK_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <stdc.h>
#include <arch/xm_def.h>

extern void InitBrk(xmAddress_t brk);
extern void *SBrk(xmSize_t incr, xm_s32_t align);

#define GET_MEM(c, s) do { \
    if (s) { \
        if (!(c=SBrk(s, ALIGNMENT))) \
            SystemPanic(0, 0, __FILE__":%u: memory pool exhausted", __LINE__); \
    } else c=0;\
} while(0)

#define GET_MEMZ(c, s) do { \
    if (s) { \
        if (!(c=SBrk(s, ALIGNMENT))) SystemPanic(0, 0, __FILE__":%u: memory pool exhausted", __LINE__); \
        memset(c, 0, s); \
    } else c=0; \
} while(0)

#endif
