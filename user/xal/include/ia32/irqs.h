/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_ARCH_IRQS_H_
#define _XAL_ARCH_IRQS_H_

#define TRAPTAB_LENGTH 256

#ifndef __ASSEMBLY__

typedef struct trapCtxt {
    xm_u32_t y;
    xm_u32_t g1;
    xm_u32_t g2;
    xm_u32_t g3;
    xm_u32_t g4;
    xm_u32_t g5;
    xm_u32_t g6;
    xm_u32_t g7;
    xm_u32_t nPc;
    xm_u32_t irqNr;
    xm_u32_t flags;
    xm_u32_t pc;
} trapCtxt_t;

#endif
#endif
