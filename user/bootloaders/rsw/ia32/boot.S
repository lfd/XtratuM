/*
 * $FILE: boot.S
 *
 * Generic boot code, sets up a valid stack and jumps to the C code
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#define STACK_ADDR 
#define STACK_SIZE (1024*16)
	
.text
.align 4

.global start, _start, _halt_system

_start:	
start:
	jmp _mboot_entry
.align 4
	
#define __MBOOT_HDR_MAGIC 0x1BADB002
#define __MBOOT_HDR_FLAGS 0x3

_MBOOT_HDR:
.long __MBOOT_HDR_MAGIC
.long __MBOOT_HDR_FLAGS
.long -(__MBOOT_HDR_MAGIC+__MBOOT_HDR_FLAGS)

_mboot_entry:
	cld
	xorl %eax,%eax
	movl $_sbss,%edi
	movl $_ebss,%ecx
	subl %edi,%ecx
	shrl $2,%ecx
	rep ; stosl

	mov $_estack, %esp
	call RSwMain

_halt_system:
        cli
        hlt
        jmp _halt_system

.bss

_stack:
	.zero STACK_SIZE
_estack:
	
.previous
