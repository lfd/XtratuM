/*
 * $FILE: bitwise.h
 *
 * Some basic bit operations
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_BITWISE_H_
#define _XM_ARCH_BITWISE_H_

#define ARCH_HAS_FFS

static __inline__ xm_s32_t _Ffs(xm_s32_t x) {
    return __builtin_ffs(x) - 1;
}

#endif
