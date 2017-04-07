/*
 * $FILE: init.c
 *
 * Initialisation of the libxm
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>

void init_arch_libxm(void) {
    extern xm_u32_t __siret[], __eiret[];
    struct trapHandler trapHndl;
    extern void LibXmGenProtTrap(void);

    XM_params_get_PCT()->arch.atomicArea.sAddr=(xm_u32_t)__siret;
    XM_params_get_PCT()->arch.atomicArea.eAddr=(xm_u32_t)__eiret;
    XMAtomicSetMask(IFLAGS_ATOMIC_MASK, &XM_params_get_PCT()->iFlags);
    trapHndl.cs=GUEST_CS;
    trapHndl.eip=(xmAddress_t)LibXmGenProtTrap;
    XM_override_trap_hndl(0xd, &trapHndl);
}
