/*
 * $FILE: head.S
 *
 * Entry point of the XM in the ia32 architecture version
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
#include <arch/irqs.h>
#include <arch/asm_offsets.h>
#include <arch/segments.h>
#include <arch/xm_def.h>
	
.section .vboot.text, "ax"
	
ENTRY(StartXM)
	/* setting up again the gdt but this time with its virtual address */
	lgdt earlyXmGdtDesc
	/* load descriptors from our new GDT */
    ljmp $(EARLY_XM_CS), $1f
	/* CS has been loaded, now it's the turn of ds, es, fs and gs */
1:	mov $(EARLY_XM_DS), %eax
	mov %eax, %ds
	mov %eax, %es
	xorl %eax, %eax
	mov %eax, %fs
	mov %eax, %gs
	
	/* loading a valid idt */
	lidt xmIdtDesc

	/* flushing tlb */
	movl %cr3, %eax
	movl %eax, %cr3

	/* now, it is high time to preparing a valid stack */
	lss sstack, %esp
	pushl $__idle_kthread /* Idle kthread */
	pushl $0 /* CPU 0 ID */

	xorl %eax,%eax		; // CPUID with 0
	cpuid

	movl %cr0, %eax
	andl $0x80000011,%eax	; // save PG,PE,ET
	orl $0x40022,%eax	; // set AM, NE and MP
	movl %eax,%cr0
	
	movl $1,%eax		; // Reading CPU features
	cpuid
	movl %edx, earlyFFlags
	
	/* clear EFLAGS */
	cld
	pushl $0
	popf
	
#ifdef CONFIG_DEBUG
	movl $0, %ebp
#endif
	/* calling our first C function */
	call Setup

	/* 
	  since the output buffer has been already initialised 
	  we can print a message on the screen 
	*/
	push $haltMsg
	call kprintf

.text
ENTRY(__HaltSystem)
	cli
	hlt
	jmp __HaltSystem

.data
PAGE_ALIGN

/* The XM's GDT */
ENTRY(earlyXmGdt)
	.quad 0x0000000000000000 /* NULL  descriptor */
	.quad 0x00cf9b000000ffff /* 1<<3 XM's code segment R0 */
	.quad 0x00cf93000000ffff /* 2<<3 XM's data segment R0 */

	.word 0
ENTRY(earlyXmGdtDesc)
	.word EARLY_XM_GDT_ENTRIES*8-1
	.long (SYMBOL_NAME(earlyXmGdt))

	.word 0
ENTRY(xmIdtDesc)
	.word IDT_ENTRIES*8-1
	.long (SYMBOL_NAME(xmIdt))

ENTRY(sstack)
	.long __idle_kthread+CONFIG_KSTACK_SIZE
	.long EARLY_XM_DS
	
haltMsg:
	.asciz "System halted."

.bss
PAGE_ALIGN
ENTRY(xmPgd)
	.zero PAGE_SIZE
	
ENTRY(xmPgt)
	/* [0 - XM_VMAP_SIZE [ */
	.zero ((XM_VMAPSIZE/(PAGE_SIZE))*4)

/* The XM's IDT */
ENTRY(xmIdt)
	.zero (IDT_ENTRIES*8)

__idle_kthread:
	.zero CONFIG_KSTACK_SIZE
