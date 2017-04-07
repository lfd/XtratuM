/*
 * $FILE: stdlib.h
 *
 * standard library functions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STDLIB_H_
#define _XAL_STDLIB_H_

#define NULL ((void *)0)

extern xm_s32_t atoi(const char* s);
extern xm_u32_t strtoul(const char *ptr, char **endptr, xm_s32_t base);
extern xm_s32_t strtol(const char *nptr, char **endptr, xm_s32_t base);
extern xm_u64_t strtoull(const char *ptr, char **endptr, xm_s32_t base);
extern xm_s64_t strtoll(const char *nptr, char **endptr, xm_s32_t base);
extern char *basename(char *path);
extern void exit(xm_s32_t status);

#endif
