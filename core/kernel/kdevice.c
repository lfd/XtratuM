/*
 * $FILE: kdevice.c
 *
 * Kernel devices
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <kdevice.h>
#include <stdc.h>
#include <processor.h>

extern struct kDevReg *kDevTab[];

void SetupKDev(void) {
    xm_s32_t e;
    for (e=0; kDevTab[e]; e++) {
	if (kDevTab[e]->Init)
	    kDevTab[e]->Init();
    }
}

const kDevice_t *LookUpKDev(const xmDev_t *dev) {
    xm_s32_t e;

    if (dev->id==XM_DEV_INVALID_ID)
	return 0;

    for (e=0; kDevTab[e]; e++)
	if (kDevTab[e]->id==dev->id) {
	    if (kDevTab[e]->GetKDev)
		return kDevTab[e]->GetKDev(dev->subId);
	    else 
		return 0;
	}
    return 0;
}

