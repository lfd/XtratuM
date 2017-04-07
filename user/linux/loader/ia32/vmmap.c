/*
 * $FILE: vmmap.c
 *
 * LinuxLoader
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <config.h>
#include <loader.h>
#include <xm.h>
#include <stdc.h>
#include <xm_inc/xmef.h>

#define PAGE_SIZE 4096

void SetupVMMap(xmAddress_t offset) {
    xm_s32_t e, i;

    i = offset >> 22;
    for (e=0; (e<i)&&(e+i<1024); e++)
	if (_pgdAddr[e])
	    XM_update_page32((xmAddress_t)&_pgdAddr[e+i], _pgdAddr[e]);    
}
