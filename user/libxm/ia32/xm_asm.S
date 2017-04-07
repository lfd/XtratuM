/*
 * $FILE: xm_asm.S
 *
 * IRET implementation
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <xm_inc/guest.h>
#include <xm_inc/arch/asm_offsets.h>

.macro SAVE_REGS
	pushl %ds	
	pushl %eax
	pushl %ecx
	pushl %edx
.endm
	
.macro RESTORE_REGS
	popl %edx
	popl %ecx
	popl %eax
	popl %ds
.endm

.macro SAVE_ALL_REGS
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

.macro RESTORE_ALL_REGS
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popl %eax
        popl %ds
        popl %es
        popl %fs
        popl %gs
.endm
	
ENTRY(XM_iret)
	/* we make room for an hypothetical pending IRQ */
	subl $8, %esp
	SAVE_REGS
	movl $GUEST_DS, %eax
	movl %eax, %ds
	
	movl %esp, %eax
	addl $16, %eax
	pushl %eax
	call XM_exec_pendirqs
	addl $4, %esp
	testl %eax, %eax
	jz __siret

	/* Emulating a new interrupt */
	RESTORE_REGS
	lret
	
	/*
	Our new frame is:
	
	-------------
	|  SS
	|  ESP
	|  EFLAGS
	|  CS
	|  EIP
	|  IFLAGS
	-------------
	|  (lret CS)
	|  (lret EIP)
	-------------
	|  DS
	|  EAX
	|  ECX                
	|  EDX	                 <--------------ESP
	*/
	
ENTRY(__siret)	
	movl 24(%esp), %edx // IFLAGS
	andl $(IFLAGS_MASK), %edx
	movl libXmParams, %eax
	movl _IFLAGS_OFFSET(%eax), %ecx	
	andl $(~IFLAGS_MASK), %ecx
	orl %edx, %ecx
	movl %ecx, _IFLAGS_OFFSET(%eax)
	RESTORE_REGS
	addl $12, %esp
	
	iret
ENTRY(__eiret)

ENTRY(LibXmGenProtTrap)
	// room to emulate lret
	subl $8, %esp
	pushl $0xd
	jmp MonitorErrCode
	
MonitorErrCode:
	cld
	SAVE_ALL_REGS
	movl $GUEST_DS, %edx
        movl %edx, %ds
        movl %edx, %es

	pushl %esp
	call LibXmMonitor
	addl $4, %esp
	testl %eax, %eax
	jz 1f
2:jmp 2b
	/* hand over the trap to Linux */
	RESTORE_ALL_REGS
	addl $4, %esp
	lret
1:
	/*
	Our new frame is:
	
	-------------
	|  SS
	|  ESP
	|  EFLAGS
	|  CS
	|  EIP
	|  IFLAGS
	-------------
	|  (lret CS)
	|  (lret EIP)
	-------------
	|  irqNr
	|  gs
        |  fs
        |  es
        |  ds
        |  eax
        |  ebp
        |  edi
        |  esi
        |  edx	
        |  ecx
        |  ebx <--------------ESP
	*/
	
	RESTORE_ALL_REGS
	addl $16, %esp
	jmp XM_iret
