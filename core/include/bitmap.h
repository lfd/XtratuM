/*
 * $FILE: bitmap.h
 *
 * Definition of a bitmap
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_BITMAP_H_
#define _XM_BITMAP_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <assert.h>
#include <brk.h>
#include <bitwise.h>
#include <stdc.h>

#define BITS_PER_U32 (32)

#define LOG2_32 (5)
#define MOD_U32_MASK ((1<<LOG2_32)-1)

#define BUFFER_SIZE(bits) \
    (((bits&MOD_U32_MASK)?(bits>>LOG2_32)+1:bits>>LOG2_32))

struct bitmap {
    xm_s32_t nbits;
    xm_u32_t *e;
};

static inline void BitmapClear(struct bitmap *b) {
    memset((xm_u8_t *)b->e, 0, sizeof(xm_u32_t)*BUFFER_SIZE(b->nbits)); 
}

static inline xm_s32_t BitmapClearBit(struct bitmap *b, xm_u32_t bp) {
    xm_u32_t b_entry=bp>>LOG2_32, bit=bp&MOD_U32_MASK;
    if (bp >= b -> nbits) return -1;
    b->e[b_entry]&=~(1 << bit);

    return 0;
}

static inline xm_s32_t BitmapClearBits(struct bitmap *b, xm_u32_t bp, xm_s32_t nb) {
    xm_u32_t b_entry=bp>>LOG2_32, bit=bp&MOD_U32_MASK;
    xm_u32_t mask=((((nb>=BITS_PER_U32)?~0:(1<<nb)-1))<<bit);
    //xm_u32_t tmp;

    if (bp>=b->nbits||(bp+nb)>b->nbits) return -1;
    //tmp=b->e[b_entry];
    b->e[b_entry++]&=~mask;
    nb-=(BITS_PER_U32-bit);

    while(nb>0) {
	mask=(((nb>=BITS_PER_U32)?~0:(1<<nb)-1));
	//tmp=b->e[b_entry];
	b->e[b_entry++]&=~mask;
	nb-=BITS_PER_U32;
    }

    return 0;
}

#endif
