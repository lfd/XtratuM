/*
 * $FILE: string.c
 *
 * String related functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdc.h>
#include <arch/xm_def.h>

#ifdef __ARCH_MEMCPY

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
