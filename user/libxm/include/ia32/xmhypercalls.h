/*
 * $FILE: xmhypercalls.h
 *
 * Arch hypercalls definition
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _ARCH_LIB_XM_HYPERCALLS_H_
#define _ARCH_LIB_XM_HYPERCALLS_H_

#ifdef _XM_KERNEL_
#error Guest file, do not include.
#endif

#include <xm_inc/config.h>
#include <xm_inc/arch/xm_def.h>
#include <xm_inc/arch/processor.h>
#include <xm_inc/arch/segments.h>

#include <xm_inc/linkage.h>

#ifndef __ASSEMBLY__

#define __DO_XMHC \
    "lcall $("TO_STR(XM_HYPERCALL_CALLGATE_SEL)"), $0x0"

#define _XM_HCALL0(_hc_nr, _r) \
    __asm__ __volatile__ (__DO_XMHC : "=a" (_r) : "0" (_hc_nr))

#define _XM_HCALL1(_a0, _hc_nr, _r) \
    __asm__ __volatile__ (__DO_XMHC : "=a" (_r) : "0" (_hc_nr), "b" ((xm_u32_t)(_a0)))

#define _XM_HCALL2(_a0, _a1, _hc_nr, _r) \
    __asm__ __volatile__ (__DO_XMHC : "=a" (_r) : "0" (_hc_nr), "b" ((xm_u32_t)(_a0)), "c" ((xm_u32_t)(_a1)))

#define _XM_HCALL3(_a0, _a1, _a2, _hc_nr, _r) \
    __asm__ __volatile__ (__DO_XMHC : "=a" (_r) : "0" (_hc_nr), "b" ((xm_u32_t)(_a0)), "c" ((xm_u32_t)(_a1)), "d" ((xm_u32_t)(_a2)))

#define _XM_HCALL4(_a0, _a1, _a2, _a3, _hc_nr, _r) \
    __asm__ __volatile__ (__DO_XMHC : "=a" (_r) : "0" (_hc_nr), "b" ((xm_u32_t)(_a0)), "c" ((xm_u32_t)(_a1)), "d" ((xm_u32_t)(_a2)), "S" ((xm_u32_t)(_a3)))

#define _XM_HCALL5(_a0, _a1, _a2, _a3, _a4, _hc_nr, _r)	\
    __asm__ __volatile__ (__DO_XMHC : "=a" (_r) : "0" (_hc_nr), "b" ((xm_u32_t)(_a0)), "c" ((xm_u32_t)(_a1)), "d" ((xm_u32_t)(_a2)), "S" ((xm_u32_t)(_a3)), "D" ((xm_u32_t)(_a4)))

#define xm_hcall0(_hc) \
__stdcall void XM_##_hc(void) { \
    xm_s32_t _r ; \
    if(XM_flush_hyp_batch()<0) return; \
    _XM_HCALL0(_hc##_nr, _r); \
}

#define xm_hcall0r(_hc) \
__stdcall xm_s32_t XM_##_hc(void) { \
    xm_s32_t _r ; \
    if((_r=XM_flush_hyp_batch())<0) return _r; \
    _XM_HCALL0(_hc##_nr, _r); \
    return _r; \
}

#define xm_hcall1(_hc, _t0, _a0) \
__stdcall void XM_##_hc(_t0 _a0) { \
    xm_s32_t _r ; \
     if(XM_flush_hyp_batch()<0) return; \
    _XM_HCALL1(_a0, _hc##_nr, _r); \
}

#define xm_hcall1r(_hc, _t0, _a0) \
__stdcall xm_s32_t XM_##_hc(_t0 _a0) { \
    xm_s32_t _r ; \
    if((_r=XM_flush_hyp_batch())<0) return _r; \
    _XM_HCALL1(_a0, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall2(_hc, _t0, _a0, _t1, _a1) \
__stdcall void XM_##_hc(_t0 _a0, _t1 _a1) { \
    xm_s32_t _r ; \
     if(XM_flush_hyp_batch()<0) return; \
    _XM_HCALL2(_a0, _a1, _hc##_nr, _r); \
}

#define xm_hcall2r(_hc, _t0, _a0, _t1, _a1) \
__stdcall xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1)  { \
    xm_s32_t _r ; \
    if((_r=XM_flush_hyp_batch())<0) return _r; \
    _XM_HCALL2(_a0, _a1, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall3(_hc, _t0, _a0, _t1, _a1, _t2, _a2) \
__stdcall void XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2) { \
    xm_s32_t _r ; \
     if(XM_flush_hyp_batch()<0) return; \
    _XM_HCALL3(_a0, _a1, _a2, _hc##_nr, _r); \
}

#define xm_hcall3r(_hc, _t0, _a0, _t1, _a1, _t2, _a2) \
__stdcall xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2) { \
    xm_s32_t _r ; \
    if((_r=XM_flush_hyp_batch())<0) return _r; \
    _XM_HCALL3(_a0, _a1, _a2, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall4(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3) \
__stdcall void XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3) { \
    xm_s32_t _r ; \
     if(XM_flush_hyp_batch()<0) return; \
    _XM_HCALL4(_a0, _a1, _a2, _a3, _hc##_nr, _r); \
}

#define xm_hcall4r(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3) \
__stdcall xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3) { \
    xm_s32_t _r ; \
    if((_r=XM_flush_hyp_batch())<0) return _r; \
    _XM_HCALL4(_a0, _a1, _a2, _a3, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall5(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3, _t4, _a4) \
__stdcall void XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3, _t4 _a4) { \
    xm_s32_t _r ; \
     if(XM_flush_hyp_batch()<0) return; \
    _XM_HCALL5(_a0, _a1, _a2, _a3, _a4, _hc##_nr, _r);  \
}

#define xm_hcall5r(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3, _t4, _a4) \
__stdcall xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3, _t4 _a4) { \
    xm_s32_t _r ; \
    if((_r=XM_flush_hyp_batch())<0) return _r; \
    _XM_HCALL5(_a0, _a1, _a2, _a3, _a4, _hc##_nr, _r);  \
    return _r; \
}

typedef struct {
    xm_u32_t high;
    xm_u32_t low;
} genericDesc_t;

extern __stdcall xm_s32_t XM_ia32_update_sys_struct(xm_u32_t procStruct, xm_u32_t val1, xm_u32_t val2);
extern __stdcall xm_s32_t XM_ia32_set_idt_desc(xm_s32_t entry, genericDesc_t *desc);
extern __stdcall void XM_lazy_ia32_update_sys_struct(xm_u32_t procStruct, xm_u32_t val1, xm_u32_t val2);

#else

 // Parameters ->
 // eax: syscall number
 // ebx: 1st parameter
 // ecx: 2nd parameter
 // ...

#define __XM_HC \
    lcall $(XM_HYPERCALL_CALLGATE_SEL), $0x0

#endif
#endif

