/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel (arch dependent part)
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <boot.h>
#include <stdc.h>
#include <physmm.h>

volatile xm_s8_t localInfoInit=0;

void __VBOOT SetupArchLocal(xm_s32_t cpuid) {
    localInfoInit=1;
}

void __VBOOT EarlySetupArchCommon(void) {
    /* There is at least one processor in the system */
    SET_NRCPUS(1);
}

void __VBOOT SetupArchCommon(void) {
}

