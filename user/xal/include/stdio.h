/*
 * $FILE: stdio.h
 *
 * stdio file
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STDIO_H_
#define _XAL_STDIO_H_

#include <stdarg.h>

extern xm_s32_t printf(const char *format,...);
extern xm_s32_t vprintf(const char *fmt, va_list args);
extern xm_s32_t sprintf(char *s, char const *fmt, ...);
extern xm_s32_t vsprintf(char *str, const char *format, va_list ap);
extern xm_s32_t putchar(xm_s32_t c);

#endif
