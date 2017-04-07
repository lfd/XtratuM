/*
 * $FILE: hypervisor.h
 *
 * hypervisor management functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _ARCH_LIB_XM_HYPERVISOR_H_
#define _ARCH_LIB_XM_HYPERVISOR_H_

#include <xm.h>

#define JMP_IHANDLER(cs, eip, eflags, iflags) \
    __asm__ __volatile__ ("pushl %%ecx\n\t" \
                          "movl %%cs, %%ecx\n\t" \
                          "pushl %%ecx\n\t" \
			  "pushl $1f\n\t"   \
			  "pushl %%ebx\n\t" \
			  "pushl %%eax\n\t" \
			  "pushl %%edx\n\t" \
			  "lret\n\t" \
			  "1:\n\t"::"a"(cs), "d"(eip), "c" (eflags), "b" (iflags))

static inline void __Irq2CsEip(xm_s32_t irq, xm_u32_t *cs, xm_u32_t *eip) {
    genericDesc_t *idttab;
    idttab=(genericDesc_t *)XM_params_get_PCT()->arch.idtr.linearBase;
    
    if (!(idttab[irq].low&(1<<8))) // irq gate
	XMAtomicClearMask(IFLAGS_IRQ_MASK, &XM_params_get_PCT()->iFlags);	
    *cs=idttab[irq].high>>16;
    *eip=(idttab[irq].high&0xFFFF)|(idttab[irq].low&0xFFFF0000);
}

static inline void __XM_partition_emul_irq(xm_u32_t irq) {
    xm_u32_t cs, eip, eflags, iflags;

    __Irq2CsEip(irq, &cs, &eip);
    iflags=XMAtomicGet(&XM_params_get_PCT()->iFlags);
    iflags=(iflags&IFLAGS_MASK)|IFLAGS_IRQ_MASK;
    __asm__ __volatile__("pushf\n\t" \
			 "popl %0\n\t" :"=r" (eflags):);
    JMP_IHANDLER(cs, eip, eflags, iflags);
}

#define __XM_partition_emul_hwIrq(irq) __XM_partition_emul_irq(XM_params_get_PCT()->hwIrq2Vector[irq])
#define __XM_partition_emul_extIrq(irq) __XM_partition_emul_irq(XM_params_get_PCT()->extIrq2Vector[irq]) 

struct ia32Ctxt {
    xm_u32_t eip;
    xm_u32_t xcs;  
};

#define __XM_partition_emul_hwIrqCtxt(irq, ctxt) __Irq2CsEip(XM_params_get_PCT()->hwIrq2Vector[irq], &((struct ia32Ctxt *)ctxt)->xcs, &((struct ia32Ctxt *)ctxt)->eip)

#define __XM_partition_emul_extIrqCtxt(irq, ctxt) __Irq2CsEip(XM_params_get_PCT()->extIrq2Vector[irq], &((struct ia32Ctxt *)ctxt)->xcs, &((struct ia32Ctxt *)ctxt)->eip)

static inline void XM_ia32_save_idtr(pseudoDesc_t *desc) {
    XM_params_get_PCT()->arch.idtr=*desc;
}

extern void init_arch_libxm(void);

#endif
