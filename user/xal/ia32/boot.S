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
#define NO_IDT_ENTRIES (256+32)
	
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

	call InitArch
	
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
	pushl 44(%esp)
        call ExceptionHandler
        addl $4, %esp
	HW_RESTORE_ALL
	jmp XM_iret
	
CommonIrqBody:
	HW_SAVE_ALL
	pushl 44(%esp)
        call HwIrqHandler
        addl $4, %esp
	HW_RESTORE_ALL
	jmp XM_iret
	
CommonExtIrqBody:
	HW_SAVE_ALL
	pushl 44(%esp)
        call ExtIrqHandler
        addl $4, %esp
	HW_RESTORE_ALL
	jmp XM_iret
	
#define TAB_ST(st) \
    .data 2; \
    .globl st ;  \
    .align 4 ; \
    st :

#define TAB_END() \
   .data 2 ; \
   .long 0 ; \
   .previous

#define BUILD_IRQ(irq) \
    .data 2 ; \
    .align 4 ; \
    .long IH##irq ; \
    .text ; \
    .align 4 ; \
    IH##irq: ; \
        pushl $0 ; \
        pushl $irq ; \
        jmp CommonIrqBody
	
#define BUILD_EXTIRQ(irq) \
    .data 2 ; \
    .align 4 ; \
    .long EIH##irq ; \
    .text ; \
    .align 4 ; \
    EIH##irq: ; \
        pushl $0 ; \
        pushl $irq ; \
        jmp CommonExtIrqBody
	
#define BUILD_TRAP_ERRCODE(trapnr) \
    .section .rodata.trapHndl,"a" ; \
    .align 4 ; \
    .long TrapHndl##trapnr ; \
    .text ; \
    .align 4 ; \
    TrapHndl##trapnr: ; \
        pushl $trapnr ; /* error_code has already been filled */ \
        jmp CommonTrapBody

#define BUILD_TRAP_NOERRCODE(trapnr) \
    .section .rodata.trapHndl,"a" ; \
    .align 4 ; \
    .long TrapHndl##trapnr ; \
    .text ; \
    .align 4 ; \
    TrapHndl##trapnr: ; \
        pushl $0 ; /* the error_code (no error code) */ \
        pushl $trapnr ;  /* the trap */ \
        jmp CommonTrapBody
	
TAB_ST(hwIrqTable)
BUILD_IRQ(0x00)
BUILD_IRQ(0x01)
BUILD_IRQ(0x02) 
BUILD_IRQ(0x03)
BUILD_IRQ(0x04) 
BUILD_IRQ(0x05)
BUILD_IRQ(0x06)
BUILD_IRQ(0x07)
BUILD_IRQ(0x08)
BUILD_IRQ(0x09) 
BUILD_IRQ(0x0a)
BUILD_IRQ(0x0b)
BUILD_IRQ(0x0c)
BUILD_IRQ(0x0d)
BUILD_IRQ(0x0e)
BUILD_IRQ(0x0f)
TAB_END()
	
TAB_ST(trapTable)
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
TAB_END()
	
/* </track id="PARTITION_BOOT_CODE"> */

TAB_ST(extIrqTable)
BUILD_EXTIRQ(0x00)
BUILD_EXTIRQ(0x01)
BUILD_EXTIRQ(0x02) 
BUILD_EXTIRQ(0x03)
BUILD_EXTIRQ(0x04) 
BUILD_EXTIRQ(0x05)
BUILD_EXTIRQ(0x06)
BUILD_EXTIRQ(0x07)
BUILD_EXTIRQ(0x08)
BUILD_EXTIRQ(0x09) 
BUILD_EXTIRQ(0x0a)
BUILD_EXTIRQ(0x0b)
BUILD_EXTIRQ(0x0c)
BUILD_EXTIRQ(0x0d)
BUILD_EXTIRQ(0x0e)
BUILD_EXTIRQ(0x0f)
BUILD_EXTIRQ(0x10)
BUILD_EXTIRQ(0x11)
BUILD_EXTIRQ(0x12)
BUILD_EXTIRQ(0x13)
BUILD_EXTIRQ(0x14)
BUILD_EXTIRQ(0x15)
BUILD_EXTIRQ(0x16)
BUILD_EXTIRQ(0x17)
BUILD_EXTIRQ(0x18)
BUILD_EXTIRQ(0x19) 
BUILD_EXTIRQ(0x1a)
BUILD_EXTIRQ(0x1b)
BUILD_EXTIRQ(0x1c)
BUILD_EXTIRQ(0x1d)
BUILD_EXTIRQ(0x1e)
BUILD_EXTIRQ(0x1f)	
TAB_END()
	
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

