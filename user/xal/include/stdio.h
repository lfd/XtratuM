/*
 * $FILE: stdio.h
 *
 * stdio file
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STDIO_H_
#define _XAL_STDIO_H_

#include <stdarg.h>

extern xm_s32_t vsnprintf(char *s, int nc, const char *fmt, va_list ap);
extern xm_s32_t vsprintf(char *str, const char *format, va_list ap);
extern xm_s32_t vprintf(const char *fmt, va_list ap);
extern xm_s32_t snprintf(char *s, xm_s32_t nc, const char *fmt, ...);
extern xm_s32_t sprintf(char *str, const char *fmt, ...);
extern xm_s32_t printf(const char *fmt, ...) ;

extern xm_s32_t putchar(xm_s32_t c);

#endif
