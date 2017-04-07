/*
 * $FILE: monitor.c
 *
 * Ia32 emulation architecture
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

struct trapCtxt_t {
    // saved registers
    xm_s32_t ebx;
    xm_s32_t ecx;
    xm_s32_t edx;
    xm_s32_t esi;
    xm_s32_t edi;
    xm_s32_t ebp;
    xm_s32_t eax;
    xm_s32_t xds;
    xm_s32_t xes;
    xm_s32_t xfs;
    xm_s32_t xgs;
    // irq or trap raised
    xm_s32_t irqNr;

    xm_u32_t emulEip;
    xm_u32_t emulXcs;

    // error code pushed by the processor, 0 if none
    xm_s32_t errorCode;
    // processor state frame
    xm_s32_t iflags;
    xm_s32_t eip;
    xm_s32_t xcs;
    xm_s32_t eflags;
    xm_s32_t esp;
    xm_s32_t xss;
};

static xm_u16_t EmulInPort(xm_u16_t port) {
    return 0;
}

static void EmulOutPort(xm_u16_t port, xm_u16_t val) {
}

xm_s32_t LibXmMonitor(struct trapCtxt_t *ctxt) {
    //extern int early_printk(const char *format, ...);
    
    xm_u8_t *inst=(xm_u8_t *)ctxt->eip;
    xm_u32_t sizeOverride=0, val;
    if (*inst==0x66) {
	sizeOverride=1;
	inst++;
    }
    
    switch(*inst) {
    case 0xe4: // IN AL, imm8
    case 0xe5: // IN EAX, imm8
	ctxt->eax=EmulInPort(*(inst+1));
	ctxt->eip+=2;
	break;	
    case 0xec: // IN AL, DL
    case 0xed: // IN EAX, DX
	if (sizeOverride) 
	    val=*(xm_u32_t *)ctxt->edx;
	else
	    val=ctxt->edx;
	ctxt->eax=EmulInPort(val);
	ctxt->eip+=1;
	break;
    case 0xe6: // OUT imm8, AL
    case 0xe7: // OUT imm8, EAX
	EmulOutPort(*(inst+1), ctxt->eax);
	ctxt->eip+=2;
	break;
    case 0xee: // OUT DX, AL
    case 0xef: // OUT DX, EAX
	EmulOutPort(ctxt->edx, ctxt->eax);
	ctxt->eip+=1;
	break;
    case 0xfa: // cli
	XM_disable_irqs();
	break;
    case 0xfb: // sti
	XM_enable_irqs();
	break;
    default:
	__Irq2CsEip(XM_params_get_PCT()->trap2Vector[ctxt->irqNr], &ctxt->emulXcs, &ctxt->emulEip);
	return 1;
    }
    return 0;
}
