/*
 * $FILE: stdc.h
 *
 * KLib's standard c functions definition
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_STDC_H_
#define _XM_STDC_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <linkage.h>


static inline xm_s32_t isspace(xm_s32_t ch) {
    return (xm_u32_t)(ch - 9) < 5u  ||  ch == ' ';
}

static inline xm_s32_t isxdigit(xm_s32_t ch) {
    return (xm_u32_t)(ch - '0') < 10u  ||
	(xm_u32_t)((ch | 0x20) - 'a') <  6u;
}

static inline xm_s32_t isalnum (xm_s32_t ch) {
    return (xm_u32_t)((ch | 0x20) - 'a') < 26u  ||
	(xm_u32_t)(ch - '0') < 10u;
}

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v,l)

#undef NULL
#define NULL ((void *) 0)

#undef OFFSETOF
#ifdef __compiler_offsetof
#define OFFSETOF(_type, _member) __compiler_offsetof(_type,_member)
#else
#define OFFSETOF(_type, _member) ((xmSize_t) &((_type *)0)->_member)
#endif

extern xm_s32_t kprintf(const char *, ...);
extern xm_s32_t vprintf(const char *fmt, va_list args);
extern xm_s32_t sprintf(char *s, char const *fmt, ...);
extern xm_s32_t snprintf(char *s, xm_s32_t n, const char *fmt, ...);
extern void *memmove(void *, const void *, xmSize_t);
extern xm_u32_t strtoul(const char *, char **, xm_s32_t);
extern xm_s32_t strtol(const char *, char **, xm_s32_t);
extern xm_s64_t strtoll(const char *nPtr, char **endPtr, xm_s32_t base);
extern xm_u64_t strtoull(const char *ptr, char **endPtr, xm_s32_t base);
extern char *basename(char *path);
extern xm_s32_t memcmp(const void *, const void *, xmSize_t);
extern void *memcpy(void *, const void *, xmSize_t);
extern void *memset(void *, xm_s32_t, xmSize_t);
extern char *strcat(char *, const char *);
extern char *strncat(char *s, const char *t, xmSize_t n);
extern char *strchr(const char *, xm_s32_t);
extern xm_s32_t strcmp(const char *, const char *);
extern xm_s32_t strncmp(const char *, const char *, xmSize_t);
extern char *strcpy(char *, const char *);
extern char *strncpy(char *dest, const char *src, xmSize_t n);
extern xmSize_t strlen(const char *);
extern char *strrchr(const char *, xm_s32_t);
extern char *strstr(const char *, const char *);

#endif
