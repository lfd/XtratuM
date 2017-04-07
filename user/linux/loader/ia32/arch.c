/*
 * $FILE: arch.c
 *
 * Architecture initialization functions
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
#include <stdc.h>

#define HwSetGate(gateAddr, type, dpl, addr) ({  \
    long __d0, __d1;		    			\
    __asm__ __volatile__ ("movw %%dx,%%ax\n\t"			\
			  "movw %4,%%dx\n\t"			\
			  "movl %%eax,%0\n\t"			\
			  "movl %%edx,%1"			\
			  :"=m" (*((long *) (gateAddr))),	\
			   "=m" (*(1+(long *) (gateAddr))), "=&a" (__d0), "=&d" (__d1) \
			  :"i" ((xm_s16_t) (0x8000+(dpl<<13)+(type<<8))), \
			  "3" ((xm_s8_t *) (addr)),"2" (((1<<3)|1) << 16)); \
})

#define HwSetIrqGate(vector, addr) 	\
    HwSetGate((xm_u32_t)idtTab+(vector)*sizeof(gateDesc_t), 14, 0, (addr))

#define HwSetTrapGate(vector, addr)	\
    HwSetGate((xm_u32_t)idtTab+(vector)*sizeof(gateDesc_t), 15, 0, (addr))

#define HwSetSysGate(vector, addr)	\
    HwSetGate((xm_u32_t)idtTab+(vector)*sizeof(gateDesc_t), 15, 3, (addr))

#define HwSetCallGate(table, sel, offset, dpl, param, segSel) do { \
    (table)[sel/8].gate = (gateDesc_t) { \
        offsetLow: ((xm_u32_t)offset & 0xFFFF), \
        selector: (segSel & 0xFFFF), \
	wordCount: (param & 0x1F), \
	access: 0x8C | ((dpl & 0x3) << 5), \
	offsetHigh: (((xm_u32_t) offset & 0xFFFF0000) >> 16) \
    }; \
} while(0)

typedef struct {
    xm_u32_t offsetLow:16,      /* offset 0..15 */
        selector:16,
        wordCount:8,
        access:8,
        offsetHigh:16;  /* offset 16..31 */
} gateDesc_t;

#define IDT_ENTRIES (256+32)
extern gateDesc_t idtTab[IDT_ENTRIES];
extern pseudoDesc_t idtDesc;

#define EXT_IRQ_VECTOR 0x90
extern partitionControlTable_t partitionControlTable;

void __attribute__ ((weak)) ExceptionHandler(xm_s32_t trapNr) {
    xprintf("exception 0x%x (%d)\n", trapNr, trapNr);
    XM_halt_partition(XM_PARTITION_SELF);
}

void __attribute__ ((weak)) ExtIrqHandler(xm_s32_t trapNr) {
    xprintf("extIrq 0x%x (%d)\n", trapNr, trapNr);
    XM_halt_partition(XM_PARTITION_SELF);
}

void __attribute__ ((weak)) HwIrqHandler(xm_s32_t trapNr) {
    xprintf("hwIrq 0x%x (%d)\n", trapNr, trapNr);
    XM_halt_partition(XM_PARTITION_SELF);
}

void InitArch(void) {
    extern void (*hwIrqTable[0])(void);
    extern void (*trapTable[0])(void);
    extern void (*extIrqTable[0])(void);
    long irqNr;

    /* Setting up the HW irqs */
    for (irqNr=0; hwIrqTable[irqNr]; irqNr++)
	HwSetIrqGate(irqNr+0x20, hwIrqTable[irqNr]);
	
	for (irqNr=0; irqNr<XM_VT_EXT_MAX; irqNr++) {
		partitionControlTable.extIrq2Vector[irqNr]=EXT_IRQ_VECTOR+irqNr;
		HwSetIrqGate(EXT_IRQ_VECTOR+irqNr, extIrqTable[irqNr]);
	}
    
    HwSetTrapGate(0, trapTable[0]);
    HwSetIrqGate(1, trapTable[1]);
    HwSetIrqGate(2, trapTable[2]);
    HwSetSysGate(3, trapTable[3]);
    HwSetSysGate(4, trapTable[4]);
    HwSetSysGate(5, trapTable[5]);
    HwSetTrapGate(6, trapTable[6]);
    HwSetTrapGate(7, trapTable[7]);
    HwSetTrapGate(8, trapTable[8]);
    HwSetTrapGate(9, trapTable[9]);
    HwSetTrapGate(10, trapTable[10]);
    HwSetTrapGate(11, trapTable[11]);
    HwSetTrapGate(12, trapTable[12]);
    HwSetTrapGate(13, trapTable[13]);
    HwSetIrqGate(13, trapTable[13]);
    HwSetIrqGate(14, trapTable[14]);
    HwSetTrapGate(15, trapTable[15]);
    HwSetTrapGate(16, trapTable[16]);
    HwSetTrapGate(17, trapTable[17]);
    HwSetTrapGate(18, trapTable[18]);
    HwSetTrapGate(19, trapTable[19]);

	partitionControlTable.arch.idtr=(pseudoDesc_t)idtDesc;
}
