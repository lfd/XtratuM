/*
 * $FILE: processor.h
 *
 * Processor functions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_PROCESSOR_H_
#define _XM_PROCESSOR_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <irqs.h>
#include <arch/processor.h>

typedef struct {
    xm_u32_t flags;
#define CPU_SLOT_ENABLED (1<<0)
#define BSP_FLAG (1<<1)
    volatile xm_u32_t irqNestingCounter;
    struct cpuArch arch;
} localCpu_t;

extern localCpu_t localCpuInfo[];
#define GET_LOCAL_CPU()	\
    (&localCpuInfo[GET_CPU_ID()])


extern void __HaltSystem(void);
extern void HaltSystem(void);
extern void DumpState(xm_s8_t from, void *regs);
#define IRQ_PANIC 1
extern void PartitionPanic(xm_s8_t from, void *regs, xm_s8_t *fmt, ...);
extern void SystemPanic(xm_s8_t from, void *regs, xm_s8_t *fmt, ...);
//extern void StackBackTrace(xm_u32_t);

extern xm_u32_t cpuKhz;
extern xm_u32_t GetCpuKhz(void);
extern void SetupCpu(void);
extern void EarlySetupCpu(void);
extern void SetupArchLocal(xm_s32_t cpuid);
extern void EarlySetupArchCommon(void);
extern void SetupArchCommon(void);
extern xm_s32_t WriteReg32(hypercallCtxt_t *ctxt, xm_s32_t reg32, xm_u32_t val);
extern xm_s32_t WriteReg64(hypercallCtxt_t *ctxt, xm_s32_t reg64, xm_u32_t high, xm_u32_t low);

#endif
