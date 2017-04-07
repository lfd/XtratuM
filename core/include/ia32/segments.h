/*
 * $FILE: segments.h
 *
 * i386 segmentation
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_SEGMENTS_H_
#define _XM_ARCH_SEGMENTS_H_

#if (CONFIG_PARTITION_NO_GDT_ENTRIES<3)
#error Number of GDT entries must be at least 3
#endif

// Early XM GDT's number of entries
#define EARLY_XM_GDT_ENTRIES 3
#define EARLY_XM_CS (1<<3) // Early XM's code segment (Ring 0)
#define EARLY_XM_DS (2<<3) // Early XM's data segment (Ring 0)

// XM GDT's number of entries
#define XM_GDT_ENTRIES 8

 // Segment selectors
#define XM_HYPERCALL_CALLGATE_SEL ((1+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3) // XM hypercall's call gate selector
#define XM_CS ((2+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3) // XM's code segment (Ring 0)
#define XM_DS ((3+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3) // XM's data segment (Ring 0)
#define TSS_SEL ((4+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3)
#define PERCPU_SEL ((5+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3)

#define GUEST_CS (((6+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3)+1) // Guest's code segment (Ring 1)
#define GUEST_DS (((7+CONFIG_PARTITION_NO_GDT_ENTRIES)<<3)+1) // Guest's data segment (Ring 1)

#ifdef _XM_KERNEL_
#ifndef __ASSEMBLY__

/* Segment descriptor: Definition */
typedef struct {
    xm_u32_t high;
    xm_u32_t low;
#define DESC_P_BIT 15
#define DESC_DPL_BIT 13
#define DESC_S_BIT 12
#define DESC_TYPE_BIT 8
} genericDesc_t;

/* trap/interrupt/call gate descriptor */
typedef struct {
    xm_u32_t offsetLow:16,	/* offset 0..15 */
	selector:16,
	wordCount:8,
	access:8,
	offsetHigh:16;	/* offset 16..31 */
} gateDesc_t;

/* Definition of a descriptor */
typedef struct {
    xm_u32_t limitLow:16,	/* limit 0..15 */
	baseLow:16, /* base  0..15 */
	baseMed:8,	/* base  16..23 */
	access:8, /* access byte */
	limitHigh:4, /* limit 16..19 */
	granularity:4, /* granularity */
	baseHigh:8; /* base 24..31 */
} desc_t;

/* Definition of an entry in the GDT table */
typedef union {
    gateDesc_t gate;
    desc_t desc;
    genericDesc_t gDesc;
} gdtDesc_t;

typedef struct {
    xm_s32_t backLink; /* segment number of previous task,
			   if nested */
    xm_s32_t esp0; /* initial stack pointer ... */
    xm_s32_t ss0; /* and segment for ring 0 */
    xm_s32_t esp1; /* initial stack pointer ... */
    xm_s32_t ss1; /* and segment for ring 1 */
    xm_s32_t esp2; /* initial stack pointer ... */
    xm_s32_t ss2; /* and segment for ring 2 */
    xm_s32_t cr3; /* CR3 - page table directory
		     physical address */
    xm_s32_t eip;
    xm_s32_t eflags;
    xm_s32_t eax;
    xm_s32_t ecx;
    xm_s32_t edx;
    xm_s32_t ebx;
    xm_s32_t esp; /* current stack pointer */
    xm_s32_t ebp;
    xm_s32_t esi;
    xm_s32_t edi;
    xm_s32_t es;
    xm_s32_t cs;
    xm_s32_t ss; /* current stack segment */
    xm_s32_t ds;
    xm_s32_t fs;
    xm_s32_t gs;
    xm_s32_t ldt; /* local descriptor table segment */
    xm_u16_t traceTrap; /* trap on switch to this task */
    xm_u16_t ioBitmapOffset;
    /* offset to start of IO permission
       bit map */
} tss_t;

typedef struct {
    tss_t t;
    xm_u32_t ioMap[2048];
} ioTss_t;

#define GetDescBase(d) (((d)->gDesc.low&0xff000000)|(((d)->gDesc.low&0xff)<<16)|((d)->gDesc.high>>16))

#define GetDescLimit(d) ((d)->gDesc.low&(1<<23))?(((((d)->gDesc.low&0xf0000)|((d)->gDesc.high&0xffff))<<12)+0xfff):(((d)->gDesc.low&0xf0000)|((d)->gDesc.high&0xffff))

#define GetTssSeg(tssDesc, b, l) do { \
    b=tssDesc.desc.baseLow|(tssDesc.desc.baseMed<<16)|(tssDesc.desc.baseHigh<<24); \
    l=tssDesc.desc.limitLow|(tssDesc.desc.limitHigh<<16); \
} while(0)

#define GetTssDesc(gdtr, tssSel) \
  ((gdtDesc_t *)((gdtr)->linearBase))[tssSel/8]

#define TssClearBusy(gdtr, tssSel) \
  GetTssDesc((gdtr), tssSel).desc.access&=~(0x2);

#define TSS_IO_MAP_DISABLED (0xFFFF)

#define DisableTssIoMap(tss)  \
  (tss)->t.ioBitmapOffset=TSS_IO_MAP_DISABLED
  
#define EnableTssIoMap(tss) \
  (tss)->t.ioBitmapOffset= \
    ((xm_u32_t)&((tss)->ioMap)-(xm_u32_t)tss)

extern void CopyIoMap(xm_u32_t *dst, xm_u32_t *src);

#define SetIoMapBit(bit, ioMap) do { \
    xm_u32_t __entry, __offset; \
    __entry=bit/32; \
    __offset=bit%32; \
    ioMap[__entry]|=(1<<__offset); \
} while(0)

#define ClearIoMapBit(bit, ioMap) do { \
    xm_u32_t __entry, __offset; \
    __entry=bit/32; \
    __offset=bit%32; \
    ioMap[__entry]&=~(1<<__offset); \
} while(0)

#define IsIoMapBitSet(bit, ioMap) do { \
    xm_u32_t __entry, __offset; \
    __entry=bit/32; \
    __offset=bit%32; \
    ioMap[__entry]&(1<<__offset);   \
} while(0)

/* XM's GDT */
extern gdtDesc_t earlyXmGdt[EARLY_XM_GDT_ENTRIES];
extern gdtDesc_t xmGdt[(CONFIG_PARTITION_NO_GDT_ENTRIES+XM_GDT_ENTRIES)*CONFIG_NO_CPUS];

extern ioTss_t xmTss[CONFIG_NO_CPUS];

#define Entry2XmGdtEntry(e) ((XM_GDT_ENTRIES*GET_CPU_ID())+(e))

/* XM's IDT */
extern gateDesc_t xmIdt[IDT_ENTRIES];
#endif
#endif

#endif
