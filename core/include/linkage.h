/*
 * $FILE: linkage.h
 *
 * Definition of some macros to ease the interoperatibility between
 * assembly and C
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_LINKAGE_H_
#define _XM_LINKAGE_H_

#ifndef _XM_KERNEL_
#include <xm_inc/arch/linkage.h>
#else
#include <arch/linkage.h>
#include <arch/paging.h>

#define PAGE_ALIGN .align PAGE_SIZE

#endif


#ifndef _ASSEMBLY_
#define __NOINLINE__ __attribute__((noinline))
#endif

#define SYMBOL_NAME(X) X
#define SYMBOL_NAME_LABEL(X) X##:

#define ENTRY(name) \
    .globl SYMBOL_NAME(name); \
    ASM_ALIGN; \
    SYMBOL_NAME_LABEL(name)

#define __STR(x) #x
#define TO_STR(x) __STR(x)

#endif
