/*
 * $FILE: pic.c
 *
 * The PC's PIC
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <irqs.h>
#include <arch/io.h>
#include <arch/pic.h>
#include <kdevice.h>

RESERVE_IOPORTS(0x20, 2);
RESERVE_IOPORTS(0xa0, 2);

#define CTLR2_IRQ 0x2

#define master_pic_iobase 0x20
#define slave_pic_iobase 0xa0
#define off_icw 0
#define off_ocw 1

#define master_icw master_pic_iobase + off_icw
#define master_ocw master_pic_iobase + off_ocw
#define slave_icw slave_pic_iobase + off_icw
#define slave_ocw slave_pic_iobase + off_ocw

#define LAST_IRQ_IN_MASTER 7

#define ICW_TEMPLATE 0x10
#define EDGE_TRIGGER 0x00
#define ADDR_INTRVL8 0x00
#define CASCADE_MODE 0x00
#define ICW4_NEEDED 0x01

#define SLAVE_ON_IR2 0x04
#define I_AM_SLAVE_2 0x02

#define SNF_MODE_DIS 0x00
#define NONBUFD_MODE 0x00
#define AUTO_EOI_MOD 0x02
#define NRML_EOI_MOD 0x00
#define I8086_EMM_MOD 0x01

#define PICM_ICW1 (ICW_TEMPLATE | EDGE_TRIGGER | ADDR_INTRVL8 | \
		   CASCADE_MODE | ICW4_NEEDED)
#define PICM_ICW3 SLAVE_ON_IR2
#define PICM_ICW4 (SNF_MODE_DIS | NONBUFD_MODE | NRML_EOI_MOD | I8086_EMM_MOD)

#define PICS_ICW1 (ICW_TEMPLATE | EDGE_TRIGGER | ADDR_INTRVL8 | \
		   CASCADE_MODE | ICW4_NEEDED)
#define PICS_ICW3 I_AM_SLAVE_2
#define PICS_ICW4 (SNF_MODE_DIS | NONBUFD_MODE | NRML_EOI_MOD | I8086_EMM_MOD)

#define NON_SPEC_EOI 0x20

static xm_u8_t cMasterMask = 0xFF, cSlaveMask = 0xFF;

static void PicEnableIrq(xm_u32_t irq) {
    if (irq <= LAST_IRQ_IN_MASTER) {
	cMasterMask=cMasterMask&~(1<<irq);
	OutBP(cMasterMask, master_ocw);
    } else {
	cMasterMask=cMasterMask&~(1<<CTLR2_IRQ);
	cSlaveMask=cSlaveMask&~(1<<(irq-LAST_IRQ_IN_MASTER-1));
	OutBP(cMasterMask, master_ocw);
	OutBP(cSlaveMask, slave_ocw);
    }
}

static void PicDisableIrq(xm_u32_t irq) {
    if (irq<=LAST_IRQ_IN_MASTER) {
	cMasterMask=cMasterMask|(1<<irq);
	OutBP(cMasterMask, master_ocw);
    } else {
	cSlaveMask=cSlaveMask|(1<<(irq-LAST_IRQ_IN_MASTER-1));
	if (!cSlaveMask) {
	    cMasterMask=cMasterMask|(1<<CTLR2_IRQ);
	    OutBP(cMasterMask, master_ocw);
	}
	OutBP(cSlaveMask, slave_ocw);
    }
}

static void PicMaskAndAckIrq(xm_u32_t irq) {
    if (irq<=LAST_IRQ_IN_MASTER) {
	cMasterMask=cMasterMask|(1<<irq);
	OutBP(cMasterMask, master_ocw);
	OutBP(NON_SPEC_EOI, master_icw);
    } else {
	cSlaveMask=cSlaveMask|(1<<(irq-LAST_IRQ_IN_MASTER-1));
	if (!cSlaveMask) {
	    cMasterMask=cMasterMask|(1<<CTLR2_IRQ);
	    OutBP(cMasterMask, master_ocw);
	}
	OutBP(cSlaveMask, slave_ocw);
	OutBP(NON_SPEC_EOI, slave_icw);
	OutBP(NON_SPEC_EOI, master_icw);
    }
}

// initialise both PICs

void InitPic(xm_u8_t master_base, xm_u8_t slave_base) {
    xm_s32_t irq;

    // initialise the master
    OutBP(PICM_ICW1, master_icw);
    OutBP(master_base, master_ocw);
    OutBP(PICM_ICW3, master_ocw);
    OutBP(PICM_ICW4, master_ocw);

    // initialise the slave
    OutBP(PICS_ICW1, slave_icw);
    OutBP(slave_base, slave_ocw);
    OutBP(PICS_ICW3, slave_ocw);
    OutBP(PICS_ICW4, slave_ocw);
  
    // ack any bogus intrs
    OutBP(NON_SPEC_EOI, master_icw);
    OutBP(NON_SPEC_EOI, slave_icw);
  
    // disable all the PIC's IRQ lines
    OutBP(cMasterMask, master_ocw);
    OutBP(cSlaveMask, slave_ocw);
  
  
    for (irq=0; irq<PIC_IRQS; irq ++) {
	hwIrqCtrl[irq]=(hwIrqCtrl_t){
	    .Enable=PicEnableIrq,
	    .Disable=PicDisableIrq,
	    .Ack=PicMaskAndAckIrq,
	    .End=PicEnableIrq,
	};
    }
}
