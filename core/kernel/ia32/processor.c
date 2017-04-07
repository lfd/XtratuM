/*
 * $FILE: processor.c
 *
 * Processor
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <boot.h>
#include <processor.h>
#include <physmm.h>
#include <sched.h>
#include <stdc.h>
#include <xmconf.h>
#include <arch/xm_def.h>
#include <arch/segments.h>
#include <arch/io.h>

/* flags returned the first time CPUID is called */
xm_u32_t earlyFFlags __VBOOTDATA=0;
gdtDesc_t xmGdt[(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES)*CONFIG_NO_CPUS];
ioTss_t xmTss[CONFIG_NO_CPUS];

typedef struct {
  xm_u32_t id; // logical ID
  xm_u32_t hwId; // HW ID
} localId_t;

static localId_t localIdTab[CONFIG_NO_CPUS];

#define PIT_CH2             	0x42
#define PIT_MODE                0x43
#define PIT_CALIBRATE_CYCLES    14551
#define PIT_CLOCKFREQ	    	1193182
#define PIT_CALIBRATE_MULT		(PIT_CLOCKFREQ/PIT_CALIBRATE_CYCLES)

__VBOOT static xm_u32_t CalibrateTscFreq(void) {
    xm_u64_t cStart, cStop;

    OutB((InB(0x61) & ~0x02) | 0x01, 0x61);
    OutB(0xb0, PIT_MODE);
    OutB(PIT_CALIBRATE_CYCLES & 0xff, PIT_CH2);
    OutB(PIT_CALIBRATE_CYCLES >> 8, PIT_CH2);
    RdTscLL(cStart);
    while ((InB(0x61) & 0x20) == 0);
    RdTscLL(cStop);

    return (cStop-cStart)*PIT_CALIBRATE_MULT;
}

xm_u32_t GetCpuKhz(void){
    xm_u32_t cpuKhz=xmcTab.hpv.cpuTab[GET_CPU_ID()].freq;
    if (cpuKhz==XM_CPUFREQ_AUTO)
        cpuKhz = CalibrateTscFreq()/1000;
	
    return cpuKhz;
}

void __VBOOT SetupCpu(void) {
    extern void HypercallHandler(void);
    xm_u16_t ldtSel=0;
    pseudoDesc_t gdtDesc;

    xmTss[GET_CPU_ID()].t.ioBitmapOffset=TSS_IO_MAP_DISABLED;

    memcpy(&xmGdt[Entry2XmGdtEntry(XM_CS>>3)], &earlyXmGdt[EARLY_XM_CS>>3], sizeof(gdtDesc_t));
    memcpy(&xmGdt[Entry2XmGdtEntry(XM_DS>>3)], &earlyXmGdt[EARLY_XM_DS>>3], sizeof(gdtDesc_t));
    
    xmGdt[Entry2XmGdtEntry(GUEST_CS>>3)].gDesc.high=((XM_OFFSET>>12)-1)&0xffff;
    xmGdt[Entry2XmGdtEntry(GUEST_CS>>3)].gDesc.low=(0x00c0bb00|(((XM_OFFSET>>12)-1)&0xf0000));
    xmGdt[Entry2XmGdtEntry(GUEST_DS>>3)].gDesc.high=((XM_OFFSET>>12)-1)&0xffff;
    xmGdt[Entry2XmGdtEntry(GUEST_DS>>3)].gDesc.low=(0x00c0b300|(((XM_OFFSET>>12)-1)&0xf0000));
        
    xmGdt[Entry2XmGdtEntry(PERCPU_SEL>>3)].desc=(desc_t) {
	.limitLow=0xFFFF,
	.baseLow=((xm_u32_t)&localIdTab[GET_CPU_ID()]&0xFFFF),
	.baseMed=(((xm_u32_t)&localIdTab[GET_CPU_ID()]>>16)&0xFF),
	.access=0x93,
	.limitHigh=0xF,
	.granularity=0xC,
	.baseHigh=(((xm_u32_t)&localIdTab[GET_CPU_ID()]>>24)&0xFF),
    };
    
    HwSetCallGate(&xmGdt[Entry2XmGdtEntry(0)], XM_HYPERCALL_CALLGATE_SEL, HypercallHandler, 2, 1, XM_CS);
    
    xmGdt[Entry2XmGdtEntry(TSS_SEL>>3)].desc=(desc_t) {
	.limitLow=(0xFFFF&(sizeof(ioTss_t)-1)),
	.baseLow=((xm_u32_t)&xmTss[GET_CPU_ID()]&0xFFFF),
	.baseMed=(((xm_u32_t)&xmTss[GET_CPU_ID()]>>16)&0xFF),
	.access=0x89,
	.limitHigh=((sizeof(ioTss_t)-1)&0xF0000)>>16,
	.granularity=0,
	.baseHigh=(((xm_u32_t)&xmTss[GET_CPU_ID()]>>24)&0xFF),
    };

    gdtDesc=(pseudoDesc_t) {
	.limit=(sizeof(gdtDesc_t)*(XM_GDT_ENTRIES+CONFIG_PARTITION_NO_GDT_ENTRIES))-1,
	.linearBase=(xm_u32_t)&xmGdt[Entry2XmGdtEntry(0)],
    };

    LoadGdt(gdtDesc);
    LoadSegSel(XM_CS, XM_DS);
    LoadLdt(ldtSel);
    LoadGs(PERCPU_SEL);
    LoadTr(TSS_SEL);
}

void __VBOOT EarlySetupCpu(void) {
    pseudoDesc_t gdtDesc;
    LoadCr4(0);
    LoadCr0(DEFAULT_CR0);
    
    if (CpuHasPse(earlyFFlags))
	SetPse();
    
    if (CpuHasPge(earlyFFlags))
	SetPge();
    
    if (earlyFFlags & 0x100) {
        kprintf("TSC stable\n");
    } else {
        kprintf("TSC unstable\n");
    }

    memcpy(&xmGdt[XM_CS>>3], &earlyXmGdt[EARLY_XM_CS>>3], sizeof(gdtDesc_t));
    memcpy(&xmGdt[XM_DS>>3], &earlyXmGdt[EARLY_XM_DS>>3], sizeof(gdtDesc_t));

    xmGdt[PERCPU_SEL>>3].desc=(desc_t) {
	.limitLow=0xFFFF,
	.baseLow=((xm_u32_t)&localIdTab[0]&0xFFFF),
	.baseMed=(((xm_u32_t)&localIdTab[0]>>16)&0xFF),
	.access=0x93,
	.limitHigh=0xF,
	.granularity=0xC,
	.baseHigh=(((xm_u32_t)&localIdTab[0]>>24)&0xFF),
    };
    gdtDesc=(pseudoDesc_t) {
	.limit=(sizeof(gdtDesc_t)*(XM_GDT_ENTRIES+CONFIG_PARTITION_NO_GDT_ENTRIES))-1,
	.linearBase=(xm_u32_t)xmGdt,
    };

    LoadGdt(gdtDesc);
    LoadSegSel(XM_CS, XM_DS);
}

#define EMPTY_SEG 0
#define CODE_DATA_SEG 1
#define TSS_SEG 2
#define LDT_SEG 3

static inline xm_s32_t IsGdtDescValid(gdtDesc_t *desc, xm_u32_t *type) {
    xm_u32_t limit, base;

    if (!(desc->gDesc.low&(1<<DESC_P_BIT))) {
	*type=CODE_DATA_SEG;
	return 1;
    }

    *type=EMPTY_SEG;
   
    limit=GetDescLimit(desc);
    base=(xm_u32_t)GetDescBase(desc);
    
    if ((limit+base)>CONFIG_XM_OFFSET) {
	 kprintf("GDT desc (0x%x:0x%x) limit too large\n", desc->gDesc.high, desc->gDesc.low);
	 return 0;
     }

     // Code/Data segment
    if (desc->gDesc.low&(1<<DESC_S_BIT)) {
	 *type=CODE_DATA_SEG;

	 // Checking the permissions
	 if (!((desc->gDesc.low>>DESC_DPL_BIT)&0x3)) {
	     //kprintf("DESCR (%x:%x) bad permissions\n", desc->gDesc.high, desc->gDesc.low);
	     return 0;
	 }

	 // Setting as accessed to avoid a MP fault (#14)
	 desc->gDesc.low|=(1<<DESC_TYPE_BIT);
     } else {
	 // System segment
	 switch ((desc->gDesc.low>>DESC_TYPE_BIT)&0xF) {
	 case 0x9:
	 case 0xb:
	     *type=TSS_SEG;  
	     break;
	     // MISS LDT
	 default:
	     return 0;
	 }
	 
	 return 1;
     }
     return 1;
}

static xm_s32_t WriteCr0(hypercallCtxt_t *ctxt, xm_u32_t val) {
    localSched_t *sched=GET_LOCAL_SCHED();
    val|=(CR0_PG|CR0_AM|CR0_PE);
    val&=~(CR0_WP|CR0_CD|CR0_NW);

    sched->cKThread->ctrl.g->kArch.cr0=sched->cKThread->ctrl.g->partitionControlTable->arch.cr0=val;

    LoadCr0(sched->cKThread->ctrl.g->kArch.cr0);

    return XM_OK;
}

static xm_s32_t WriteCr3(hypercallCtxt_t *ctxt, xm_u32_t val) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct physPage *newCr3Page;
    xm_u32_t oldCr3;

    SavePgd(oldCr3);
    if (oldCr3==val) {
	FlushTlb();
    } else {
	if (!(newCr3Page=PmmFindPage(val, sched->cKThread, 0)))
	    return XM_INVALID_PARAM;
	if (newCr3Page->type!=PPAG_PGD) {
	    kprintf("Page %x is not PGD\n", val&PAGE_MASK);
	    return XM_INVALID_PARAM;
	}
	sched->cKThread->ctrl.g->kArch.pgd=val;
	sched->cKThread->ctrl.g->partitionControlTable->arch.cr3=val;
	LoadPgd(val);
    }
 
    return XM_OK;
}

static xm_s32_t WriteCr4(hypercallCtxt_t *ctxt, xm_u32_t val) {
    localSched_t *sched=GET_LOCAL_SCHED();
    val&=~(CR4_PAE);
    val|=(CR4_PSE|CR4_PGE);

    sched->cKThread->ctrl.g->kArch.cr4=sched->cKThread->ctrl.g->partitionControlTable->arch.cr4=val;

    LoadCr4(sched->cKThread->ctrl.g->kArch.cr4);
    return XM_OK;

}

static xm_s32_t WriteTss(hypercallCtxt_t *ctxt, tss_t *__gParam t) {
    localSched_t *sched=GET_LOCAL_SCHED();

    if (__CheckGParam(ctxt, t, sizeof(tss_t))<0) 
	return XM_INVALID_PARAM;

    if (!(t->ss1&0x3)||((t->ss1>>3)>=(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES))) return XM_INVALID_PARAM;

    if (t->esp1>=CONFIG_XM_OFFSET) return XM_INVALID_PARAM;

    if (!(t->ss2&0x3)||((t->ss2>>3)>=(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES))) return XM_INVALID_PARAM;

    if (t->esp2>=CONFIG_XM_OFFSET) return XM_INVALID_PARAM;

    sched->cKThread->ctrl.g->kArch.tss.ss1=t->ss1;
    xmTss[GET_CPU_ID()].t.ss1=t->ss1;
    sched->cKThread->ctrl.g->kArch.tss.esp1=t->esp1;
    xmTss[GET_CPU_ID()].t.esp1=t->esp1;
    sched->cKThread->ctrl.g->kArch.tss.ss2=t->ss2;
    xmTss[GET_CPU_ID()].t.ss2=t->ss2;
    sched->cKThread->ctrl.g->kArch.tss.esp2=t->esp2;
    xmTss[GET_CPU_ID()].t.esp2=t->esp2;

    return XM_OK;
}

static xm_s32_t WriteTlb(hypercallCtxt_t *ctxt, xm_u32_t val) {
    if (val==-1)
	FlushTlb();
    else
	FlushTlbEntry(val);

    return XM_OK;
}

static xm_s32_t Wbinvd(hypercallCtxt_t *ctxt, xm_u32_t val) {
    __asm__ __volatile__("wbinvd": : :"memory");
    return XM_OK;
}

static xm_s32_t WriteGdt(hypercallCtxt_t *ctxt, pseudoDesc_t *__gParam desc) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t gdtNoEntries, e, type;
    gdtDesc_t *gdt;

    if (__CheckGParam(ctxt, desc, sizeof(pseudoDesc_t))<0) 
	return XM_INVALID_PARAM;
    
    gdtNoEntries=(desc->limit+1)/sizeof(gdtDesc_t);
    if (gdtNoEntries>CONFIG_PARTITION_NO_GDT_ENTRIES) 
	return XM_INVALID_PARAM;

    sched->cKThread->ctrl.g->partitionControlTable->arch.gdtr=*desc;    
    memset(sched->cKThread->ctrl.g->kArch.gdtTab, 0, CONFIG_PARTITION_NO_GDT_ENTRIES*sizeof(gdtDesc_t));
    for (e=0, gdt=(gdtDesc_t *)desc->linearBase; e<gdtNoEntries; e++)
	if (IsGdtDescValid(&gdt[e], &type)&&(type==CODE_DATA_SEG)) {
	    sched->cKThread->ctrl.g->kArch.gdtTab[e].gDesc=gdt[e].gDesc;
	    xmGdt[e].gDesc=gdt[e].gDesc;
	}

    return XM_OK;
}

/* This enumeration must be sync with the include/ia32/processor.h */
xm_s32_t (*Reg32HndlTab[NR_REGS32])(hypercallCtxt_t *, xm_u32_t)={
    [CR0_REG32]=WriteCr0,
    [CR3_REG32]=WriteCr3,
    [CR4_REG32]=WriteCr4,
    [GDT_REG32]=(xm_s32_t(*)(hypercallCtxt_t *, xm_u32_t))WriteGdt,
    [TSS_REG32]=(xm_s32_t(*)(hypercallCtxt_t *, xm_u32_t))WriteTss,
    [TLB_REG32]=WriteTlb,
    [WBINVD_REG32]=Wbinvd,
};

static xm_s32_t Reg64_RdMsr(hypercallCtxt_t *ctxt, xm_u32_t msr, xm_u32_t valH, xm_u32_t valL) {
    localSched_t *sched=GET_LOCAL_SCHED();

    if(sched->cKThread->ctrl.g->cfg->id)
		return XM_INVALID_PARAM;
    if (__CheckGParam(ctxt, valH, sizeof(valH))<0) 
		return XM_INVALID_PARAM;
    if (__CheckGParam(ctxt, valL, sizeof(valL))<0) 
		return XM_INVALID_PARAM;

    RdMsr(msr, *(xm_u32_t*)valL, *(xm_u32_t*)valH);
    return XM_OK;
}

static xm_s32_t Reg64_WrMsr(hypercallCtxt_t *ctxt, xm_u32_t msr, xm_u32_t valH, xm_u32_t valL) {
    localSched_t *sched=GET_LOCAL_SCHED();

    if(sched->cKThread->ctrl.g->cfg->id)
		return XM_INVALID_PARAM;
    WrMsr(msr, valL, valH); 
    return XM_OK;
}

/* This enumeration must be sync with the include/ia32/processor.h */
xm_s32_t (*Reg64HndlTab[NR_REGS32])(hypercallCtxt_t *, xm_u32_t, xm_u32_t, xm_u32_t)={
    [RDMSR_REG64] = Reg64_RdMsr,
    [WRMSR_REG64] = Reg64_WrMsr,
};

static xm_s32_t Ia32UpdateSsEsp1(hypercallCtxt_t *ctxt, xm_u32_t ss1, xm_u32_t esp1) {
    localSched_t *sched=GET_LOCAL_SCHED();

    if (!(ss1&0x3)||((ss1>>3)>=(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES))) return XM_INVALID_PARAM;

    if (esp1>=CONFIG_XM_OFFSET) return XM_INVALID_PARAM;

    sched->cKThread->ctrl.g->kArch.tss.ss1=ss1;
    xmTss[GET_CPU_ID()].t.ss1=ss1;
    sched->cKThread->ctrl.g->kArch.tss.esp1=esp1;
    xmTss[GET_CPU_ID()].t.esp1=esp1;
    return XM_OK;
}

static xm_s32_t Ia32UpdateSsEsp2(hypercallCtxt_t *ctxt, xm_u32_t ss2, xm_u32_t esp2) {
    localSched_t *sched=GET_LOCAL_SCHED();

    if (!(ss2&0x3)||((ss2>>3)>=(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES))) return XM_INVALID_PARAM;

    if (esp2>=CONFIG_XM_OFFSET) return XM_INVALID_PARAM;

    sched->cKThread->ctrl.g->kArch.tss.ss2=ss2;
    xmTss[GET_CPU_ID()].t.ss2=ss2;
    sched->cKThread->ctrl.g->kArch.tss.esp2=esp2;
    xmTss[GET_CPU_ID()].t.esp2=esp2;
    return XM_OK;
}

static xm_s32_t Ia32UpdateGdt(hypercallCtxt_t *ctxt, xm_s32_t entry, gdtDesc_t *__gParam gdt) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_u32_t type;

    if (__CheckGParam(ctxt, gdt, sizeof(gdtDesc_t))<0) 
	return XM_INVALID_PARAM;

    if (entry>=CONFIG_PARTITION_NO_GDT_ENTRIES) 
	return XM_INVALID_PARAM;

    if (IsGdtDescValid(gdt, &type)&&(type==CODE_DATA_SEG)) {
	sched->cKThread->ctrl.g->kArch.gdtTab[entry].gDesc=gdt->gDesc;
	xmGdt[entry].gDesc=gdt->gDesc;
    } else
	return XM_INVALID_PARAM;

    return XM_OK;
}

xm_s32_t (*UpdPrcStructHndlTab[NR_IA32_PROC_STRUCT])(hypercallCtxt_t *, xm_u32_t, xm_u32_t)={
    [IA32_UPDATE_SSESP1]=Ia32UpdateSsEsp1,
    [IA32_UPDATE_SSESP2]=Ia32UpdateSsEsp2,
    [IA32_UPDATE_GDT]=(xm_s32_t (*)(hypercallCtxt_t *, xm_u32_t, xm_u32_t))Ia32UpdateGdt,
};
