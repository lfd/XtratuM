/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_IRQS_H_
#define _XM_IRQS_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#ifndef __ASSEMBLY__

#include <linkage.h>
#include <arch/irqs.h>

#define SCHED_PENDING 0x80000000
#define IRQ_IN_PROGRESS (~SCHED_PENDING)

typedef xm_s32_t (*irqHandler_t)(irqCtxt_t *, void *);
typedef xm_s32_t (*trapHandler_t)(irqCtxt_t *); // Returns a HM event

struct irqTabEntry {
    irqHandler_t handler;
    void *data;
};

/*
  struct irq2pin {
  unsigned long flags;
  #define IRQ_TRIGGER_MODE_BIT 0
  #define IRQ_TRIGGER_MODE_MASK (1<<IRQ_TRIGGER_MODE_BIT)
  #define   EDGE_MODE 0
  #define   LEVEL_MODE 1
  #define IRQ_PIN_POLARITY_BIT 1
  #define IRQ_PIN_POLARITY_MASK (1<<IRQ_PIN_POLARITY_BIT)
  #define   HIGH_ACT 0
  #define   LOW_ACT 1
  #define IRQ_EXTRA_BIT 2
  xm_s32_t pin; //, next;
  #define NO_PIN -1
  #define INTERNAL_IRQ -2
  };
*/

#if (HWIRQ_NR)>32
#error HWIRQ_NR is greater than 32
#endif
#if (TRAP_NR)>32
#error TRAP_NR is greater than 32
#endif

extern struct irqTabEntry irqHandlerTab[HWIRQ_NR];
extern trapHandler_t trapHandlerTab[TRAP_NR];
extern void SetupIrqs(void);
extern irqHandler_t SetIrqHandler(xm_s32_t, irqHandler_t, void *);
extern trapHandler_t SetTrapHandler(xm_s32_t, trapHandler_t);

extern void ArchSetupIrqs(void);

// Control over each interrupt
typedef struct {
    void (*Enable)(xm_u32_t irq);
    void (*Disable)(xm_u32_t irq);
    void (*Ack)(xm_u32_t irq);
    void (*End)(xm_u32_t irq);
} hwIrqCtrl_t;

extern hwIrqCtrl_t hwIrqCtrl[HWIRQ_NR];

static inline void HwDisableIrq(xm_s32_t irq) {
    if ((irq<HWIRQ_NR)&&hwIrqCtrl[irq].Disable)
	hwIrqCtrl[irq].Disable(irq);
}

static inline void HwEnableIrq(xm_s32_t irq) {
    if ((irq<HWIRQ_NR)&&hwIrqCtrl[irq].Enable)
	hwIrqCtrl[irq].Enable(irq);
}

static inline void HwAckIrq(xm_s32_t irq) {
    if ((irq<HWIRQ_NR)&&hwIrqCtrl[irq].Ack)
	hwIrqCtrl[irq].Ack(irq);
}

static inline void HwEndIrq(xm_s32_t irq) {
    if ((irq<HWIRQ_NR)&&hwIrqCtrl[irq].End)
	hwIrqCtrl[irq].End(irq);
}

extern xm_s32_t MaskHwIrq(xm_s32_t irq);
extern xm_s32_t UnmaskHwIrq(xm_s32_t irq);
extern void SetTrapPending(irqCtxt_t *ctxt);

#endif

#endif
