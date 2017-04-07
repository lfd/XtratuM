/*
 * $FILE: string.h
 *
 * string functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STRING_H_
#define _XAL_STRING_H_

#define NULL ((void *)0)

extern xm_s32_t atoi(const char* s);

extern void *memset(void *dst, xm_s32_t s, xm_u32_t count);
extern void *memcpy(void *dst, const void* src, xm_u32_t count);
extern xm_s32_t memcmp(const void *dst, const void *src, xm_u32_t count);
extern char *strcpy(char *dst, const char *src);
extern char *strcat(char *s, const char* t);
extern char *strncat(char *s, const char *t, xmSize_t n);
extern xm_s32_t strcmp(const char *s, const char *t);
extern xm_s32_t strncmp(const char *s1, const char *s2, xmSize_t n);
extern xm_u32_t strlen(const char *s);
extern char *strchr(const char *t, xm_s32_t c);

#endif
