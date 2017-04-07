/*
 * $FILE: processor.h
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

#ifndef _ARCH_XM_PROCESSOR_H_
#define _ARCH_XM_PROCESSOR_H_

/* x86 reachable registers */

/* <track id="IA32-REG32"> */
#define CR0_REG32 0
#define CR3_REG32 1
#define CR4_REG32 2
#define GDT_REG32 3
#define TSS_REG32 4
#define TLB_REG32 5
#define WBINVD_REG32 6
#define NR_REGS32 7
/* </track id="IA32-REG32"> */

/*#define LDT_REG32 4
#define DB0_REG32 6
#define DB1_REG32 7
#define DB2_REG32 8
#define DB3_REG32 9
#define DB4_REG32 10
#define DB5_REG32 11
#define DB6_REG32 12
#define DB7_REG32 13*/

#define RDMSR_REG64 0
#define WRMSR_REG64 1
#define NR_REGS64 2

/* <track id="IA32-UPDATE-REG"> */
#define IA32_UPDATE_SSESP1 0
#define IA32_UPDATE_SSESP2 1
#define IA32_UPDATE_GDT 2

#define NR_IA32_PROC_STRUCT 3
/* </track id="IA32-UPDATE-REG"> */

#ifdef _XM_KERNEL_

/* EFLAGS' flags */
#define CPU_FLAG_TF     0x00000100
#define CPU_FLAG_IF     0x00000200
#define CPU_FLAG_IOPL   0x00003000
#define CPU_FLAG_NT     0x00004000
#define CPU_FLAG_VM     0x00020000
#define CPU_FLAG_AC     0x00040000
#define CPU_FLAG_VIF    0x00080000
#define CPU_FLAG_VIP    0x00100000
#define CPU_FLAG_ID     0x00200000

#define CPU_EFLAGS_IF_BIT 9
#define CPU_EFLAGS_IOPL_BIT 12

#define CR0_PE (1<<0)
#define CR0_TS (1<<3)
#define CR0_WP (1<<16)
#define CR0_AM (1<<18)
#define CR0_NW (1<<29)
#define CR0_CD (1<<30)
#define CR0_PG (1<<31)

#define CR4_VME (1<<0)
#define CR4_PSE (1<<4)
#define CR4_PAE (1<<5)
#define CR4_PGE (1<<7)

#define DEFAULT_CR0 0x80040033
#define DEFAULT_CR4 0x90

#endif

#ifndef __ASSEMBLY__

typedef struct __attribute__ ((packed)) {
    xm_u16_t limit;
    xm_u32_t linearBase __attribute__((packed));
} pseudoDesc_t;

#ifdef _XM_KERNEL_

extern pseudoDesc_t xmGdtDesc, xmIdtDesc;

#define HwSaveStack(stack)					\
    __asm__ __volatile__ ("movl %%esp, %0\n\t" : "=r" (stack))

#define RdTscLL(count) \
    __asm__ __volatile__("rdtsc" : "=A" (count))

#define RdTsc(lowcount,highcount)						\
    __asm__ __volatile__("rdtsc" : "=a" (lowcount), "=d" (highcount))

#define RdMsr(msr,val1,val2)				\
    __asm__ __volatile__("rdmsr"			\
			 : "=a" (val1), "=d" (val2)	\
			 : "c" (msr))

#define RdMsrl(msr,val) do {				\
	xm_u32_t a__,b__;				\
	__asm__ __volatile__("rdmsr"			\
			     : "=a" (a__), "=d" (b__)	\
			     : "c" (msr));		\
	val = a__ | (b__<<32);				\
    } while(0)

#define WrMsr(msr,val1,val2)					\
    __asm__ __volatile__("wrmsr"				\
			 : /* no outputs */			\
			 : "c" (msr), "a" (val1), "d" (val2))

#define WrMsrl(msr,val) WrMsr(msr,(__u32)((__u64)(val)),((__u64)(val))>>32)

#define SaveEip(eip) do { \
    __asm__ __volatile__ ("movl $1f, %0\n\t"	\
  		          "1:\n\t" : "=r"(eip));	\
} while(0)

#define SaveEbp(ebp) do {  \
    __asm__ __volatile__ ("mov %%ebp, %0\n\t" : "=a" (ebp));	\
} while(0)

#define SaveCs(cs) do {	\
    __asm__ __volatile__ ("mov %%cs, %0\n\t" : "=a" (cs));	\
} while(0)

#define SaveSs(ss) do {	\
    __asm__ __volatile__ ("mov %%ss, %0\n\t" : "=a" (ss));	\
} while(0)

#define SaveGs(gs) do {	\
    __asm__ __volatile__ ("mov %%gs, %0\n\t" : "=a" (gs));	\
} while(0)

#define LoadDs(ds) do {	\
    __asm__ __volatile__ ("mov %0, %%ds\n\t" : : "r" (ds)); \
} while(0)

#define LoadEs(es) do {	\
    __asm__ __volatile__ ("mov %0, %%es\n\t" : : "r" (es)); \
} while(0)

#define LoadFs(fs) do {	\
    __asm__ __volatile__ ("mov %0, %%fs\n\t" : : "r" (fs)); \
} while(0)

#define LoadGs(gs) do {	\
    __asm__ __volatile__ ("mov %0, %%gs\n\t" : : "r" (gs)); \
} while(0)

#define ClearDs() do {	\
    __asm__ __volatile__ ("xorl %%eax, %%eax\n\t" \
                          "movl %%eax, %%ds\n\t" : :);	\
} while(0)

#define ClearEs() do {	\
    __asm__ __volatile__ ("xorl %%eax, %%eax\n\t" \
                          "movl %%eax, %%es\n\t" : :);	\
} while(0)

#define ClearFs() do { \
    __asm__ __volatile__ ("xorl %%eax, %%eax\n\t" \
			  "movl %%eax, %%fs\n\t" : :);	\
} while(0)

#define ClearGs() do { \
    __asm__ __volatile__ ("xorl %%eax, %%eax\n\t" \
                          "movl %%eax, %%gs\n\t" : :);	\
} while(0)

#define SaveCr0(cr0) do {  \
    __asm__ __volatile__ ("movl %%cr0, %0\n\t" : "=r" (cr0)); \
} while(0)

#define LoadCr0(cr0) do {  \
    __asm__ __volatile__ ("movl %0, %%cr0\n\t" : : "r" (cr0));	\
} while(0)

#define SaveCr2(cr2) do { \
    __asm__ __volatile__ ("movl %%cr2, %0\n\t" : "=r" (cr2));	\
} while(0)

#define LoadCr2(cr2) do { \
    __asm__ __volatile__ ("movl %0, %%cr2\n\t" : : "r" (cr2));	\
} while(0)

/*
  #define SaveCr3(cr3) do { \
  __asm__ __volatile__ ("movl %%cr3, %0\n\t" : "=r" (cr3)); \
  } while(0)

  #define LoadCr3(cr3) do { \
  __asm__ __volatile__ ("movl %0, %%cr3\n\t" : : "r" (cr3)); \
  } while(0)
*/

#define SaveCr4(cr4) do {  \
    __asm__ __volatile__ ("movl %%cr4, %0\n\t" : "=r" (cr4));	\
} while(0)

#define LoadCr4(cr4) do {  \
    __asm__ __volatile__ ("movl %0, %%cr4\n\t" : : "r" (cr4));	\
} while(0)

static inline void CpuId(xm_u32_t op, xm_u32_t *eax, xm_u32_t *ebx, xm_u32_t *ecx, xm_u32_t *edx) {
    *eax=op;
    *ecx=0;
    __asm__ __volatile__ ("cpuid\n\t" : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), \
			  "=d" (*edx) : "0" (*eax), "2" (*ecx));
}

static inline xm_u32_t GetCpuIdEbx(xm_u32_t op) {
    xm_u32_t eax, ebx;
  
    __asm__ __volatile__ ("cpuid\n\t" : "=a" (eax), "=b" (ebx) \
			  : "0" (op) : "cx", "dx");
    return ebx;
}

static inline xm_u32_t GetCpuIdEdx(xm_u32_t op) {
    xm_u32_t eax, edx;
  
    __asm__ __volatile__ ("cpuid\n\t" : "=a" (eax), "=d" (edx) \
			  : "0" (op) : "bx", "cx");
    return edx;
}

#define SetVme() do {	\
    xm_u32_t tmpreg;				\
    __asm__ __volatile__ ("movl %%cr4, %0\n\t" : "=r" (tmpreg)); \
    tmpreg |= CR4_VME; \
	__asm__ __volatile__ ("movl %0, %%cr4\n\t" : : "r" (tmpreg));	\
} while(0)

#define SetPse() do {  \
    xm_u32_t tmpreg;				\
    __asm__ __volatile__ ("movl %%cr4, %0\n\t" : "=r" (tmpreg));  \
    tmpreg |= CR4_PSE;	\
    __asm__ __volatile__ ("movl %0, %%cr4\n\t" : : "r" (tmpreg));	\
} while(0)

#define SetPge() do {					\
    xm_u32_t tmpreg;				\
    __asm__ __volatile__ ("movl %%cr4, %0\n\t" : "=r" (tmpreg)); \
    tmpreg |= CR4_PGE;				\
    __asm__ __volatile__ ("movl %0, %%cr4\n\t" : : "r" (tmpreg));	\
} while(0)

#define SetWp() do {					\
    xm_u32_t tmpreg;				\
    __asm__ __volatile__ ("movl %%cr0, %0\n\t": "=r" (tmpreg));	 \
    tmpreg |= CR0_WP;				\
    __asm__ __volatile__ ("movl %0, %%cr0\n\t" : : "r" (tmpreg));	\
} while(0)

#define ClearWp() do {	\
    xm_u32_t tmpreg; \
    __asm__ __volatile__ ("movl %%cr0, %0\n\t": "=r" (tmpreg)); \
    tmpreg &= (~CR0_WP); \
	__asm__ __volatile__ ("movl %0, %%cr0\n\t" : : "r" (tmpreg)); \
} while(0)

#define CpuHasPse(__fflags) (__fflags&(1<<3))

#define CpuHasPge(__fflags) (__fflags&(1<<13))

#define CpuHasLApic(__fflags) (__fflags&(1<<9))

#define LoadTr(seg) \
    __asm__ __volatile__ ("ltr %0\n\t" : : "rm" ((xm_u16_t)(seg)))

#define SaveTr(seg) \
    __asm__ __volatile__ ("str %0\n\t" : "=r" ((xm_u16_t)(seg)))


#define SaveGdt(gdt) \
    __asm__ __volatile__ ("sgdt %0\n\t" : "=m" (gdt) :)

#define LoadGdt(gdt) \
    __asm__ __volatile__ ("lgdt %0\n\t" : : "m" (gdt))

#define SaveLdt(ldt) \
    __asm__ __volatile__ ("sldt %0\n\t" : "=m" (ldt) :)

#define LoadLdt(ldt) \
    __asm__ __volatile__ ("lldt %0\n\t" : : "m" (ldt))

#define SaveIdt(idt) \
    __asm__ __volatile__ ("sidt %0\n\t" : "=m" (idt) :)

#define LoadIdt(idt) \
    __asm__ __volatile__ ("lidt %0\n\t" : : "m" (idt))

#define HwSetGate(gateAddr, type, dpl, addr) ({  \
    xm_s32_t __d0, __d1;		    			\
    __asm__ __volatile__ ("movw %%dx,%%ax\n\t"			\
			  "movw %4,%%dx\n\t"			\
			  "movl %%eax,%0\n\t"			\
			  "movl %%edx,%1"			\
			  :"=m" (*((xm_s32_t *) (gateAddr))),	\
			   "=m" (*(1+(xm_s32_t *) (gateAddr))), "=&a" (__d0), "=&d" (__d1) \
			  :"i" ((xm_s16_t) (0x8000+(dpl<<13)+(type<<8))), \
			   "3" ((xm_s8_t *) (addr)),"2" (XM_CS << 16)); \
})

#define HwSetIrqGate(vector, addr) 	\
    HwSetGate((xm_u32_t)xmIdt+(vector)*sizeof(gateDesc_t), 14, 0, (addr))

#define HwSetTrapGate(vector, addr)	\
    HwSetGate((xm_u32_t)xmIdt+(vector)*sizeof(gateDesc_t), 15, 0, (addr))

#define HwSetSysGate(vector, addr)	\
    HwSetGate((xm_u32_t)xmIdt+(vector)*sizeof(gateDesc_t), 15, 3, (addr))

#define HwSetCallGate(table, sel, offset, dpl, param, segSel) do { \
    (table)[sel/8].gate = (gateDesc_t) { \
        offsetLow: ((xm_u32_t)offset & 0xFFFF), \
        selector: (segSel & 0xFFFF), \
	wordCount: (param & 0x1F), \
	access: 0x8C | ((dpl & 0x3) << 5), \
	offsetHigh: (((xm_u32_t) offset & 0xFFFF0000) >> 16) \
    }; \
} while(0)

#define LoadPgd(pgd) \
    __asm__ __volatile__ ("movl %0,%%cr3": :"r" (pgd))

#define SavePgd(pgd)	\
    __asm__ __volatile__ ("movl %%cr3, %0\n\t": "=r" (pgd) :)

#define FlushTlb() do {	\
    xm_u32_t tmpreg; \
    __asm__ __volatile__("movl %%cr3, %0;  # flush TLB \n\t"	\
		         "movl %0, %%cr3;\n\t" : "=r" (tmpreg) :: "memory"); \
} while(0)

#define FlushTlbGlobal() do { \
    xm_u32_t cr4, cr4_a; \
    __asm__ __volatile__ ("movl %%cr4, %0\n\t" : "=r" (cr4)); \
    cr4_a = cr4 & (~CR4_PGE);			\
    __asm__ __volatile__ ("movl %0,%%cr4\n\t" : : "r" (cr4_a));	\
    FlushTlb();  \
    __asm__ __volatile__ ("movl %0,%%cr4\n\t" : : "r" (cr4)); \
} while(0)

#define FlushTlbEntry(addr) \
    __asm__ __volatile__("invlpg (%0)" ::"r" (addr) : "memory")

#define LoadSegSel(_cs, _ds) \
    __asm__ __volatile__ ("ljmp $"TO_STR(_cs)", $1f\n\t" \
			  "1:\n\t" \
			  "movl $("TO_STR(_ds)"), %%eax\n\t" \
			  "mov %%eax, %%ds\n\t" \
			  "mov %%eax, %%es\n\t" \
			  "mov %%eax, %%ss\n\t" ::)

#define DoNop() __asm__ __volatile__ ("nop\n\t" ::)

#endif

/* Structure used to perform a ljmp or a lcall */
typedef struct {
    xm_u32_t offset;
    xm_u16_t sel;
} ljmpSeg_t;

struct cpuArch {
    xm_u32_t fflags;
};

#define HwCli() __asm__ __volatile__ ("cli\n\t":::"memory")
#define HwSti() __asm__ __volatile__ ("sti\n\t":::"memory")

#define HwRestoreFlags(flags) \
    __asm__ __volatile__("pushl %0\n\t"				\
			 "popfl\n\t": :"g" (flags):"memory")

#define HwSaveFlags(flags) \
    __asm__ __volatile__("pushfl\n\t" \
 		         "popl %0\n\t" :"=g" (flags): :"memory")

#define HwSaveFlagsCli(flags) { \
    HwSaveFlags(flags); \
    HwCli();  \
}

static inline xm_s32_t HwIsSti(void) {
    xm_u32_t flags;
    HwSaveFlags(flags);
    return (flags&0x200);
}

#define GET_NRCPUS() 1
#define SET_NRCPUS(nrCpu)

#define GET_CPU_ID() 0
#define GET_CPU_HWID() 0

#endif

#endif
