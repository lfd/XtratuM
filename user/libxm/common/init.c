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

struct libXmParams libXmParams;

__stdcall void init_libxm(partitionControlTable_t *partCtrlTab, partitionInformationTable_t *partInfTab) {
    extern void XM_init_batch(void);
    libXmParams.partCtrlTab=partCtrlTab;
    libXmParams.partInfTab=partInfTab;
    init_batch();
    init_arch_libxm();
}

