/*
 * $FILE: lazy.c
 *
 * Deferred hypercalls
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

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v,l)

#define XM_BATCH_LEN 1024

static volatile xm_u32_t xmHypercallBatch[XM_BATCH_LEN];
static volatile xm_s32_t batchLen=0, prevBatchLen=0;

__stdcall xm_s32_t XM_flush_hyp_batch(void) {
    if (batchLen) {
	xm_u32_t r;
	if((r=XM_multicall((void *)xmHypercallBatch, (void *)&xmHypercallBatch[batchLen]))<0) return r;
	prevBatchLen=batchLen;
	batchLen=0;
    }
    return XM_OK;
}

#define __xm_restore_iflags(__iflags) XMAtomicSet(&libXmParams.partCtrlTab->iFlags, __iflags)
#define __xm_save_iflags_cli(__iflags) do { \
    __iflags=XMAtomicGet(&libXmParams.partCtrlTab->iFlags); \
    XMAtomicClearMask(IFLAGS_IRQ_MASK, &libXmParams.partCtrlTab->iFlags); \
} while(0)

__stdcall void XM_lazy_hypercall(xm_u32_t noHyp, xm_s32_t noArgs, ...) {
    va_list args;
    xm_s32_t e;
    xm_u32_t flags;

    __xm_save_iflags_cli(flags);
    if ((batchLen>=XM_BATCH_LEN)||((batchLen+noArgs+2)>=XM_BATCH_LEN))
        XM_flush_hyp_batch();
    xmHypercallBatch[batchLen++]=noHyp;
    xmHypercallBatch[batchLen++]=noArgs;
    va_start(args, noArgs);
    for (e=0; e<noArgs; e++)
        xmHypercallBatch[batchLen++]=va_arg(args, xm_u32_t);
    va_end(args);
    __xm_restore_iflags(flags);
}

void init_batch(void) {
    batchLen=0;
    prevBatchLen=0;
}
