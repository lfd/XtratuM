/*
 * $FILE: brk.c
 *
 * Memory for structures
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

/* Here the idea is simple, we calculate how much memory is going to
   be required, reserving it as a static buffer */

#include <brk.h>
#include <xmconf.h>

#define ALIGN_SIZE(size, align) ((((~(size))+1)&((align)-1))+(size))

static xm_u8_t *brk;

void InitBrk(xmAddress_t brkPtr) {
    brk=(void *)brkPtr;
}

void *SBrk(xmSize_t incr, xm_s32_t align) {
    void *_brk;

    brk=(xm_u8_t *)ALIGN_SIZE((xm_u32_t)brk, align);

    if (((xmAddress_t)brk+incr)>=(XM_OFFSET+xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].startAddr+xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size)) return 0;
    _brk=brk;
    brk+=incr;

    return _brk;
}
