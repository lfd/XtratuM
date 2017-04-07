/*
 * $FILE: stdio.c
 *
 * Standard buffered input/output
 *
 * $VERSION$
 *
 * Author: Salva Peiró <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _STDC_H_
#define _STDC_H_

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v,l)

extern void *memcpy(void *dst, const void *src, xm_u32_t count);
extern void xputchar(xm_s32_t c);
extern xm_s32_t xprintf(const char *fmt, ...);

extern void InitOutput(void);

#endif
