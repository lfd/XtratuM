/*
 * $FILE: string.c
 *
 * String related functions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdc.h>
#include <arch/xm_def.h>

#include <arch/paging.h>
#include <processor.h>

#ifdef __ARCH_MEMCPY

void *MemcpyPhys(void *dst, const void *src, xm_u32_t count) {
    extern xm_u32_t xmPhys[], _sphys[];
    extern void _MemcpyPhys(void *dst, const void *src, xm_u32_t count);
    xmAddress_t *pgTab, *s, *d;
    xm_u32_t backup;

    SaveCr3(pgTab);
    backup=pgTab[XMVIRT2PHYS(xmPhys)>>PGDIR_SHIFT];
    pgTab[XMVIRT2PHYS(xmPhys)>>PGDIR_SHIFT]=_PG_PRESENT|_PG_RW|XMVIRT2PHYS(xmPhys);
    xmPhys[VA2Pgt((xm_u32_t)_sphys)]=_PG_PRESENT|_PG_RW|_PG_GLOBAL|(xm_u32_t)_sphys;

    s = (xmAddress_t *)PageTranslate((xmAddress_t)src);
    if (!s)
        s = (void *)src;
    d = (xmAddress_t *)PageTranslate((xmAddress_t)dst);
    if (!d)
        d = dst;
    _MemcpyPhys(d, s, count);

    pgTab[XMVIRT2PHYS(xmPhys)>>PGDIR_SHIFT]=backup;

    return dst;
}

static inline void *_MemCpy1(xm_s8_t *dest, const xm_s8_t *src, xm_u32_t n) {
    xm_s8_t *dp = dest;
    const xm_s8_t *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

static inline void *_MemCpy2(xm_s16_t *dest, const xm_s16_t *src, xm_u32_t n) {
    xm_s16_t *dp = dest;
    const xm_s16_t *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

static inline void *_MemCpy4(xm_s32_t *dest, const xm_s32_t *src, xm_u32_t n) {
    xm_s32_t *dp = dest;
    const xm_s32_t *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

static inline void *_MemCpy8(xm_s64_t *dest, const xm_s64_t *src, xm_u32_t n) {
    xm_s64_t *dp = dest;
    const xm_s64_t *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

void *memcpy(void *dst, const void *src, xm_u32_t count) {
    xm_s8_t *d, *s;
    xm_s32_t r;
    d=(xm_s8_t *)dst;
    s=(xm_s8_t *)src;
    // Are both buffers aligned to 8 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&7)) {
	r=count&7;
	_MemCpy8((xm_s64_t *)d, (xm_s64_t *)s, count>>3);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }
    // Are both buffers aligned to 4 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&3)) {
	r=count&3;
	_MemCpy4((xm_s32_t *)d, (xm_s32_t *)s, count>>2);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }
    // Are both buffers aligned to 2 bytes?
    if (!(((xm_u32_t)dst|(xm_u32_t)src)&1)) {
	r=count&1;
	_MemCpy2((xm_s16_t *)d, (xm_s16_t *)s, count>>1);
	if (!r)
	    return dst;
	d=&d[count-r];
	s=&s[count-r];
	count=r;
    }

    // Copying the buffers byte per byte
    return _MemCpy1(d, s, count);
}

#endif
