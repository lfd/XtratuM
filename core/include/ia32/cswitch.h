/*
 * $FILE: cswitch.h
 *
 * Processor
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_CSWITCH_H_
#define _XM_ARCH_CSWITCH_H_
#include <arch/asm_offsets.h>

#ifndef __ASSEMBLY__

#define PUSH_REGISTERS \
    "pushl %%eax\n\t" \
    "pushl %%ebp\n\t" \
    "pushl %%edi\n\t" \
    "pushl %%esi\n\t" \
    "pushl %%edx\n\t" \
    "pushl %%ecx\n\t" \
    "pushl %%ebx\n\t"

#define FP_OFFSET 108

#define PUSH_FP  \
    "sub $"TO_STR(FP_OFFSET)",%%esp\n\t" \
    "fnsave (%%esp)\n\t" \
    "fwait\n\t"

#define POP_FP  \
    "frstor (%%esp)\n\t" \
    "add $"TO_STR(FP_OFFSET)", %%esp\n\t"

#define POP_REGISTERS \
    "popl %%ebx\n\t" \
    "popl %%ecx\n\t" \
    "popl %%edx\n\t" \
    "popl %%esi\n\t" \
    "popl %%edi\n\t" \
    "popl %%ebp\n\t" \
    "popl %%eax\n\t"

#define CONTEXT_SWITCH(nKThread, cKThread) \
    __asm__ __volatile__(PUSH_REGISTERS \
			 PUSH_FP  \
                         "movl (%%ebx), %%edx\n\t" \
                         "pushl $1f\n\t" \
                         "movl %%esp, "TO_STR(_KSTACK_OFFSET)"(%%edx)\n\t" \
                         "movl "TO_STR(_KSTACK_OFFSET)"(%%ecx), %%esp\n\t" \
                         "movl %%ecx, (%%ebx)\n\t" \
                         "ret\n\t" \
                         "1:\n\t" \
			 POP_FP \
			 POP_REGISTERS \
			 : :"c" (nKThread), "b" (cKThread))

#define JMP_PARTITION(entry) \
    __asm__ __volatile__ ("pushl $"TO_STR(GUEST_DS)"\n\t" /* SS */ \
                          "pushl $0\n\t" /* ESP */ \
			  "pushl $"TO_STR(CPU_FLAG_IF|0x2)"\n\t" /* EFLAGS */ \
			  "pushl $"TO_STR(GUEST_CS)"\n\t" /* CS */ \
                          "pushl %0\n\t" /* EIP */ \
			  "movl $"TO_STR(GUEST_DS)", %%edx\n\t"	\
			  "movl %%edx, %%ds\n\t" \
			  "movl %%edx, %%es\n\t" \
			  "xorl %%edx, %%edx\n\t" \
			  "movl %%edx, %%fs\n\t" \
			  "movl %%edx, %%gs\n\t" \
			  "movl %%edx, %%ebp\n\t" \
			  "iret" : : "r" (entry): "edx")

#endif

#endif
