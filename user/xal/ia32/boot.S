/*
 * $FILE: boot.S
 *
 * Generic boot code, sets up a valid stack and jumps to the C code
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 * Modified: Salva Peiró <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

/* <track id="PARTITION_BOOT_CODE"> */

#include <xm.h>
#include <xm_inc/arch/asm_offsets.h>

#define NO_PGTS 8
#define NO_IDT_ENTRIES 256
#define NO_HWIRQS 16
#define NO_EXTIRQS 32
	
.align 4
.global __xmPartitionHdr
__xmPartitionHdr:
        .long XMEF_PARTITION_HDR_MAGIC
        .long start
        .long __xmImageHdr
        .long partitionControlTable
        .long partitionInformationTable
	.long _pgdAddr
	.long (NO_PGTS+1)*4096
	
.text
.align 4

.global start, _start

_start:	
start:
	cld
	xorl %eax,%eax
	movl $_sbss,%edi
	movl $_ebss,%ecx
	subl %edi,%ecx
	shrl $2,%ecx
	rep ; stosl
	
	mov $_estack, %esp

	mov $write_register32_nr, %eax
	mov $GDT_REG32, %ebx
	mov $gdtDesc, %ecx
	__XM_HC

	ljmpl $((1<<3)|1), $1f
1:

	mov $((2<<3)|1), %ebx
	mov %ebx, %ds
	mov %ebx, %ss
	mov %ebx, %gs
	mov %ebx, %fs

	pushl (__xmPartitionHdr+_PARTITIONINFORMATIONTABLE_OFFSET)
	pushl (__xmPartitionHdr+_PARTITIONCONTROLTABLE_OFFSET)
	call init_libxm
	addl $8, %esp
	call InitArch

	call PartitionMain

	mov $halt_partition_nr, %eax
	mov (partitionInformationTable+_ID_OFFSET), %ebx
	__XM_HC
1:
        jmp 1b

#define HW_SAVE_ALL \
    cld ; \
    pushl %gs ; \
    pushl %fs ; \
    pushl %es ; \
    pushl %ds ; \
    pushl %eax ; \
    pushl %ebp ; \
    pushl %edi ; \
    pushl %esi ; \
    pushl %edx ; \
    pushl %ecx ; \
    pushl %ebx	
	
#define HW_RESTORE_ALL  \
    popl %ebx ; \
    popl %ecx ; \
    popl %edx ; \
    popl %esi ; \
    popl %edi ; \
    popl %ebp ; \
    popl %eax ; \
    popl %ds ; \
    popl %es ; \
    popl %fs ; \
    popl %gs ; \
    addl $8, %esp
	
CommonTrapBody:
	HW_SAVE_ALL
	pushl %esp
	call DoTrap
	addl $4, %esp
	HW_RESTORE_ALL
	jmp XM_iret

.macro TABLE_START section, symbol
.section .rodata.\section\(),"a"
.globl \symbol\()
.align 4
\symbol\() :
.endm

.macro TABLE_END section
.section .rodata.\section\(),"a"
.align 4
.long 0
.previous
.endm
	
.macro BUILD_TRAP_ERRCODE trap
.section .rodata.trapHndl,"a"
.align 4
.long 1f
.text
.align 4
1:
       	pushl $\trap\() /* error_code has already been filled */
       	jmp CommonTrapBody
.endm

.macro BUILD_TRAP_NOERRCODE trap
.section .rodata.trapHndl,"a"
.align 4
.long 1f
.text
.align 4
1:
       	pushl $0 /* the error_code (no error code) */
       	pushl $\trap\()  /* the trap */
       	jmp CommonTrapBody
.endm

.macro BUILD_TRAP_BLOCK stTrapNr endTrapNr
vector=\stTrapNr\()
.rept \endTrapNr\()-\stTrapNr\()
	BUILD_TRAP_NOERRCODE vector
vector=vector+1
.endr
.endm
	
TABLE_START trapHndl, trapTable
BUILD_TRAP_NOERRCODE(0x0)
BUILD_TRAP_NOERRCODE(0x1)
BUILD_TRAP_NOERRCODE(0x2)
BUILD_TRAP_NOERRCODE(0x3)
BUILD_TRAP_NOERRCODE(0x4) 
BUILD_TRAP_NOERRCODE(0x5)
BUILD_TRAP_NOERRCODE(0x6) 
BUILD_TRAP_NOERRCODE(0x7)
BUILD_TRAP_ERRCODE(0x8)
BUILD_TRAP_NOERRCODE(0x9)
BUILD_TRAP_ERRCODE(0xa) 
BUILD_TRAP_ERRCODE(0xb)
BUILD_TRAP_ERRCODE(0xc) 
BUILD_TRAP_ERRCODE(0xd)
BUILD_TRAP_ERRCODE(0xe)
BUILD_TRAP_NOERRCODE(0xf)
BUILD_TRAP_NOERRCODE(0x10) 
BUILD_TRAP_ERRCODE(0x11)
BUILD_TRAP_NOERRCODE(0x12)
BUILD_TRAP_NOERRCODE(0x13)
BUILD_TRAP_ERRCODE(0x14) 
BUILD_TRAP_ERRCODE(0x15)
BUILD_TRAP_ERRCODE(0x16)	
BUILD_TRAP_ERRCODE(0x17)
BUILD_TRAP_ERRCODE(0x18) 
BUILD_TRAP_ERRCODE(0x19)
BUILD_TRAP_ERRCODE(0x1a) 
BUILD_TRAP_ERRCODE(0x1b)
BUILD_TRAP_ERRCODE(0x1c) 
BUILD_TRAP_ERRCODE(0x1d)
BUILD_TRAP_ERRCODE(0x1e) 
BUILD_TRAP_ERRCODE(0x1f)
BUILD_TRAP_BLOCK 0x20 NO_IDT_ENTRIES
TABLE_END trapHndl
	
.data
	.word 0
.global idtDesc
.align 4
idtDesc:
	.word NO_IDT_ENTRIES*8-1
	.long idtTab
	
gdtTab:
	.quad 0x0000000000000000 /* NULL  descriptor */
        .quad 0x00cfba000000bfff /* 1<<3 code segment R1 */
        .quad 0x00cfb2000000bfff /* 2<<3 data segment R1 */

 .word 0
ENTRY(gdtDesc)
        .word 3*8-1
        .long gdtTab
	
.bss

_stack:
        .zero STACK_SIZE
_estack:

.globl idtTab
idtTab:
	.zero (NO_IDT_ENTRIES*8)

.section .xm_ctrl, "w"
.align 4096
_pgdAddr:
	.zero 4096
	.zero (4096*NO_PGTS)
	
.previous

