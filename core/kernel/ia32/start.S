/*
 * $FILE: start.S
 *
 * The execution of XM starts from this file (start symbol)
 *
 * The actions performed are:
 *  1) Set up a valid GDT
 *  2) Set up a valid page table
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <linkage.h>
#include <xmef.h>
#include <hypercalls.h>
#include <arch/asm_offsets.h>
#include <arch/paging.h>
#include <arch/segments.h>
#include <arch/processor.h>
#include <arch/xm_def.h>

#define __VIRT2PHYS(X) (X-XM_OFFSET)

.section .xm_header, "ax"
ASM_ALIGN
.long XMEF_XM_MAGIC                                                      // Signature
.long XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION) // XM's ABI version
.long XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION) // XM's ABI version
.long 0                                                                  // imageId
.long 0                                                                  // checksum
.long __VIRT2PHYS(_sxm)                                                  // sAddr
.long __VIRT2PHYS(_exm)                                                  // eAddr
.long start                                                              // ePoint
.long 1                                                                  // noModules
.long __VIRT2PHYS(xmcTab)                                                // xmc file StartAddr
.long 0                                                                  // xmc file size (undefined)
	
.section .boot.text, "ax"

ASM_ALIGN

ENTRY(start)
ENTRY(_start)
	/* zero BSS */
	xorl %eax, %eax
	movl $__VIRT2PHYS(_sbss), %edi
	movl $__VIRT2PHYS(_ebss), %ecx
	subl %edi, %ecx
	rep
	stosb

	/* setting up a valid gdt */
	lgdt _gdtDesc

	 /* loading our new gdt's descriptors */
	ljmp $(EARLY_XM_CS), $1f
1:	mov $(EARLY_XM_DS), %eax
	mov %eax, %ds
	mov %eax, %es
	xorl %eax, %eax
	mov %eax, %fs
	mov %eax, %gs

	/* Only XM is mapped */
	movl $__VIRT2PHYS(xmPgt), %edi
	movl $__VIRT2PHYS((XM_OFFSET+8*1024*1024)-PAGE_SIZE), %ecx // 4Mb
	
	andl $PGT_MASK, %ecx
	sarl $(PAGE_SHIFT-2), %ecx
	add $__VIRT2PHYS(xmPgt), %ecx
	
	/* PGT's flags: SUPERVISOR + RW + PRESENT + GLOBAL */
	movl $(_PG_PRESENT|_PG_RW), %eax
2:	stosl
	add $PAGE_SIZE, %eax
	cmp %ecx, %edi
	jne 2b
	
	movl $__VIRT2PHYS(xmPgt), %ebx
	
	/* this PGD's flags: SUPERVISOR + RW + PRESENT */
	add $(_PG_PRESENT|_PG_RW), %ebx
	
	/* mapping the first four physical MB in the PGD */
	xorl %eax, %eax
	mov %ebx, __VIRT2PHYS(xmPgd)(, %eax, 4)
	
	/* filling the PGD with the real PGT */
	movl $__VIRT2PHYS(xmPgd)+(XM_PGD_START * 4), %edi

	/* PGD's flags: SUPERVISOR+RW+PRESENT */
	movl $(_PG_PRESENT|_PG_RW)+__VIRT2PHYS(xmPgt), %eax

5:	stosl
	add $0x1000, %eax
	cmp $__VIRT2PHYS(xmPgd) + (XM_PGD_END*4), %edi
	jne 5b

	/* enabling paging */
        movl $__VIRT2PHYS(xmPgd), %eax
        movl %eax, %cr3
        movl %cr0,%eax
        orl $CR0_PG, %eax
        movl %eax,%cr0

	/* 
	  once paging is enabled, it is possible to jump
          to the actual xm's enter point
	*/

	jmp StartXM

#if 0
__halt_beeping:
        mov    $0xb6,%al
        mov    $0x43,%edx
        out    %al,(%dx)
        xor    %eax,%eax
        mov    $0x42,%edx
        out    %al,(%dx)
        mov    $0x6,%al
        out    %al,(%dx)
        mov    $0x61,%edx
        in     (%dx),%al
        or     $0x3,%al
        out    %al,(%dx)
1:      hlt
        jmp 1b
#endif	
.section .boot.data, "ax"
        .word 0
_gdtDesc:
        .word EARLY_XM_GDT_ENTRIES*8-1
        .long (__VIRT2PHYS(earlyXmGdt))
