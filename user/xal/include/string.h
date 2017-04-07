/*
 * $FILE: string.h
 *
 * string functions
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STRING_H_
#define _XAL_STRING_H_

extern void *memset(void *dst, xm_s32_t s, xm_u32_t count);
extern void *memcpy(void *dst, const void* src, xm_u32_t count);
extern xm_s32_t memcmp(const void *dst, const void *src, xm_u32_t count);
extern char *strcpy(char *dst, const char *src);
extern char *strncpy(char *dest, const char *src, xmSize_t n);
extern char *strcat(char *s, const char* t);
extern char *strncat(char *s, const char *t, xmSize_t n);
extern xm_s32_t strcmp(const char *s, const char *t);
extern xm_s32_t strncmp(const char *s1, const char *s2, xmSize_t n);
extern xm_u32_t strlen(const char *s);
extern char *strrchr(const char *t, xm_s32_t c);
extern char *strchr(const char *t, xm_s32_t c);
extern char *strstr(const char *haystack, const char *needle);
extern void *memmove(void *dst, const void *src, xmSize_t count);

#endif
