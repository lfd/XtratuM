/*
 * $FILE: virtmm.h
 *
 * Virtual memory manager
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_VIRTMM_H_
#define _XM_VIRTMM_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

extern void SetupVirtMM(xmAddress_t sAddr, xmAddress_t eAddr);
extern xmAddress_t VmmAlloc(xm_s32_t npag);
extern void VmmPrintMap(void);
extern xm_s32_t VmmIsFree(xmAddress_t vAddr);
extern xm_s32_t VmmGetNoFreeFrames(void);

#endif
