/*
 * $FILE: vmmap.h
 *
 * Virtual memory map manager
 *
 * $VERSION$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_VMMAP_H_
#define _XM_VMMAP_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

extern void SetupVmMap(void);
extern xmAddress_t SetupPartitionPgd(kThread_t *k, struct xmPartitionHdr *xmPHdr);
extern void VmMapPage(xmAddress_t pAddr, xmAddress_t vAddr, xm_u32_t flags);
extern void VmUnmapPage(xmAddress_t vAddr);

#define ROUNDUP2PAGE(addr, _pS) ((((~(addr)) + 1)&((_pS)-1))+(addr))
#define ROUNDDOWN2PAGE(addr, _pS) ((addr)&~((_pS)-1))
#define SIZE2PAGES(size) \
    (((((~(size)) + 1) & (PAGE_SIZE - 1)) + (size))>>PAGE_SHIFT)

#endif
