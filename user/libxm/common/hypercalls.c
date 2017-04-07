/*
 * $FILE: hypercalls.c
 *
 * XM system calls definitions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xmhypercalls.h>
#include <xm_inc/hypercalls.h>
#include <hypervisor.h>

__stdcall xm_lazy_hcall2(write_register32, xm_u32_t, reg32, xm_u32_t, val);
__stdcall xm_lazy_hcall4(write_register64, xm_u32_t, reg64, xm_u32_t, sreg64, xm_u32_t, valH, xm_u32_t, valL);
__stdcall xm_lazy_hcall2(update_page32, xmAddress_t, pAddr, xm_u32_t, val);
__stdcall xm_lazy_hcall2(set_page_type, xmAddress_t, pAddr, xm_u32_t, type);
