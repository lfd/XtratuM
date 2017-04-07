/*
 * $FILE: stdarg.h
 *
 * std arguments
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STDARG_H_
#define _XAL_STDARG_H_

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v,l)

#endif
