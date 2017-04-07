/*
 * $FILE: entry.S
 *
 * Interrupts' code
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
#include <arch/processor.h>
#include <arch/segments.h>
#include <arch/xm_def.h>
#include <hypercalls.h>

// interrupt context when an irq has been triggered and
// HW_SAVE_ALL has been executed
/*
  -------------
68|  SS
64|  ESP
60|  EFLAGS
56|  CS
52|  EIP
  -------------
48|  error_code
44|  irqnr
40|  GS
36|  FS
32|  ES
28|  DS
24|  EAX
20|  EBP
16|  EDI
12|  ESI
 8|  EDX
 4|  ECX
 0|  EBX         <------ ESP
  -------------- <- IRQ_CONTEXT
  |  ...
*/

// syscall context when a syscall has been invoked and
// HW_SAVE_ALL has been executed and EFLAGS has been correctly stored
/*
  -------------
60|  SS
56|  ESP
52|  EFLAGS
48|  CS
44|  EIP
  -----------
40|  GS
36|  FS
32|  ES
28|  DS
24|  EAX
20|  EBP
16|  EDI
12|  ESI
 8|  EDX
 4|  ECX
 0|  EBX         <------ ESP
  -------------- <- IRQ_CONTEXT
  |  ...
*/

// interrupt context when an IPI has been triggered and
// HW_SAVE_ALL has been executed
/*
  -------------
64|  SS
60|  ESP
56|  EFLAGS
52|  CS
48|  EIP
  -------------
44|  ipi_code
40|  GS
36|  FS
32|  ES
28|  DS
24|  EAX
20|  EBP
16|  EDI
12|  ESI
 8|  EDX
 4|  ECX
 0|  EBX         <------ ESP
  -------------- <- IRQ_CONTEXT
  |  ...
*/
	
#define CS_IC_SP 56
#define CS_IPI_SP 52
	
#define EFLAGS_SC_SP 52
#define EAX_SP 24

#define IRET_FRAME_SC_SP 44
#define IRET_FRAME_IC_SP 52
	
#ifdef CONFIG_DEBUG
#define ROLLBACK_STACK movl $0, %ebp
#else
#define ROLLBACK_STACK
#endif

.macro SET_WP
	movl %cr0, %eax
	orl $(CR0_WP), %eax
	movl %eax, %cr0
.endm
	
.macro CLEAR_WP
	movl %cr0, %eax
	andl $(~(CR0_WP)), %eax
	movl %eax, %cr0
.endm
	
.macro HW_SAVE_REGS
	pushl %gs
	pushl %fs
	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
.endm
	
.macro HW_RESTORE_REGS
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
1:  	popl %ds
2:  	popl %es
3:  	popl %fs
4:  	popl %gs
.section .gp_ex_tab, "a"
	.align 4
	.long 1b
	.long 2b
	.long 3b
	.long 4b
.previous
.endm
	
.macro HW_SAVE_ALL
	cld
	HW_SAVE_REGS
	movl $(XM_DS), %edx
	movl %edx, %ds
	movl %edx, %es
	xorl %edx, %edx
	movl %edx, %fs
	movl $(PERCPU_SEL), %edx
	movl %edx,%gs
.endm
	
.macro HW_RESTORE_ALL
	HW_RESTORE_REGS
	addl $8, %esp
.endm

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
	
/* BUILD_IRQ stores in the stack, the triggered irq 
   and 0 (no error code) */

.macro BUILD_IRQ irq
.section .rodata.irqHndl,"a"
.align 4
.long 1f
.text
.align 4
1:
       	pushl $0
       	pushl $\irq\()
       	jmp CommonIrqBody
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

// HW irq routines	
TABLE_START irqHndl, hwIrqHndlTab
vector=0
.rept CONFIG_NO_HWIRQS
	BUILD_IRQ vector
vector=vector+1
.endr
TABLE_END irqHndl
	
// Traps routines
TABLE_START trapHndl, trapHndlTab
BUILD_TRAP_NOERRCODE 0x0
BUILD_TRAP_NOERRCODE 0x1
BUILD_TRAP_NOERRCODE 0x2
BUILD_TRAP_NOERRCODE 0x3
BUILD_TRAP_NOERRCODE 0x4 
BUILD_TRAP_NOERRCODE 0x5
BUILD_TRAP_NOERRCODE 0x6 
BUILD_TRAP_NOERRCODE 0x7
BUILD_TRAP_ERRCODE 0x8
BUILD_TRAP_NOERRCODE 0x9
BUILD_TRAP_ERRCODE 0xa 
BUILD_TRAP_ERRCODE 0xb
BUILD_TRAP_ERRCODE 0xc 
BUILD_TRAP_ERRCODE 0xd
BUILD_TRAP_ERRCODE 0xe
BUILD_TRAP_NOERRCODE 0xf
BUILD_TRAP_NOERRCODE 0x10 
BUILD_TRAP_ERRCODE 0x11
BUILD_TRAP_NOERRCODE 0x12 
BUILD_TRAP_NOERRCODE 0x13
BUILD_TRAP_ERRCODE 0x14 
BUILD_TRAP_ERRCODE 0x15
BUILD_TRAP_ERRCODE 0x16	
BUILD_TRAP_ERRCODE 0x17
BUILD_TRAP_ERRCODE 0x18 
BUILD_TRAP_ERRCODE 0x19
BUILD_TRAP_ERRCODE 0x1a 
BUILD_TRAP_ERRCODE 0x1b
BUILD_TRAP_ERRCODE 0x1c 
BUILD_TRAP_ERRCODE 0x1d
BUILD_TRAP_ERRCODE 0x1e 
BUILD_TRAP_ERRCODE 0x1f
TABLE_END trapHndl

.macro CHECK_USER_CTXT label, offset
	movl \offset\()(%esp), %eax
	andl $0x3, %eax
	testl %eax, %eax
	jz \label\()
.endm

.macro COMMON_BODY hndlr
.text
.align 4
Common\hndlr\()Body:
       	HW_SAVE_ALL
       	ROLLBACK_STACK
       	CHECK_USER_CTXT 1f, CS_IC_SP
       	CLEAR_WP 
1:      pushl %esp
       	call Do\hndlr\()
       	addl $4, %esp
       	CHECK_USER_CTXT 1f, CS_IC_SP
	pushl %esp
	call __EmulIrq
	addl $4, %esp
       	SET_WP
1:	HW_RESTORE_ALL
       	iret
.endm
	
COMMON_BODY Irq
COMMON_BODY Trap

.text
.align 4
/* XM's system calls */
ENTRY(HypercallHandler)
    cli
    HW_SAVE_REGS
    movl $(XM_DS),%edx
    movl %edx,%ds
    movl %edx,%es
    xorl %edx,%edx
    movl %edx,%fs
    movl $(PERCPU_SEL),%edx
    movl %edx,%gs
    ROLLBACK_STACK	
    CLEAR_WP
// fixing the stack frame (EFLAGS hasn't been stored yet)
    pushf
    popl %eax
    orl $(CPU_FLAG_IF), %eax
    movl %eax, EFLAGS_SC_SP(%esp)
    cld
// done
    movl EAX_SP(%esp), %eax
    cmpl $NR_HYPERCALLS, %eax
    jae 1f
    movl hypercallsTab(, %eax, 4), %eax
    testl %eax, %eax
    jz 1f
    pushl %esp
    call *%eax
    addl $4, %esp
    movl %eax, EAX_SP(%esp)
    jmp 2f
1:  pushl EAX_SP(%esp)
    pushl $unknownHypercallStr
    call kprintf
    addl $0x8, %esp
    movl $-1, EAX_SP(%esp)

2:
    SET_WP
    HW_RESTORE_REGS
    iret
	
.text
.align 4

ENTRY(UnexpectedIrq)
    pushl $0x0
    pushl $0xff
    HW_SAVE_ALL
    ROLLBACK_STACK
    CHECK_USER_CTXT 1f, CS_IC_SP
    CLEAR_WP
1:  pushl $unexpecIrqStr
    call kprintf
    addl $4, %esp
    CHECK_USER_CTXT 1f, CS_IC_SP
    SET_WP
1:  HW_RESTORE_ALL
    iret

.data
	
unexpecIrqStr:
    .asciz "Unexpected irq\n"
	
unknownHypercallStr:
    .asciz "Hypercall (%d) unknown\n"

unknownIpiStr:
    .asciz "IPI (%d) unkown\n"
