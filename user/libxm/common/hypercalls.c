/*
 * $FILE: hypercalls.c
 *
 * XM system calls definitions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xmhypercalls.h>
#include <xm_inc/hypercalls.h>
#include <hypervisor.h>

xm_lazy_hcall2(write_register32, xm_u32_t, reg32, xm_u32_t, val);
xm_lazy_hcall4(write_register64, xm_u32_t, reg64, xm_u32_t, sreg64, xm_u32_t, valH, xm_u32_t, valL);
xm_lazy_hcall2(update_page32, xmAddress_t, pAddr, xm_u32_t, val);
xm_lazy_hcall2(set_page_type, xmAddress_t, pAddr, xm_u32_t, type);
