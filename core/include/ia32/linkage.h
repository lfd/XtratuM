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

#ifndef _XM_ARCH_LINKAGE_H_
#define _XM_ARCH_LINKAGE_H_

#ifndef _ASSEMBLY_
#define ALIGNMENT 4
#define ASM_ALIGN .align ALIGNMENT
#define __stdcall __attribute__((regparm(0)))
//#define __fastcall
#define __hypercall __attribute__((noinline, regparm(0), section(".text")))
#define ALIGNED_C __attribute__((aligned(4)))
#endif

#endif
