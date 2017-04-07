/*
 * $FILE: physmm.h
 *
 * Physical memory manager
 *
 * $VERSION$
 * 
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_PHYSMM_H_
#define _XM_PHYSMM_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <bitmap.h>
#include <kthread.h>
#include <processor.h>
#include <hypercalls.h>
#include <arch/physmm.h>
#include <arch/paging.h>

#ifdef CONFIG_MMU
struct physPage {
    struct dynListNode listNode;
    xmAddress_t vAddr;
    xm_u32_t mapped:1, unlocked:1, type:3, counter:27;
};
#else
struct physPage {
};
#endif

#define PPAG_INC_COUNT(_p) do { \
    if ((_p)->counter==~0) \
        PartitionPanic(0, 0, __FILE__":%u: counter overflow", __LINE__); \
    (_p)->counter++; \
} while(0)

#define PPAG_DEC_COUNT(_p) do { \
    if (!((_p)->counter)) \
        PartitionPanic(0, 0, __FILE__":%u: counter <0", __LINE__); \
    (_p)->counter--; \
} while(0)

extern void SetupPhysMM(void);
extern void ZeroPhysMM(void);
extern struct physPage *PmmFindPage(xmAddress_t pAddr, kThread_t *k, xm_u32_t *flags);
extern xm_s32_t PmmFindAddr(xmAddress_t pAddr, kThread_t *k, xm_u32_t *flags);
extern void *VCacheMapPage(xmAddress_t pAddr, struct physPage *page);
extern void VCacheUnlockPage(struct physPage *page);

#endif
