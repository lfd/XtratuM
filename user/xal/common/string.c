/*
 * $FILE: string.c
 *
 * String related functions
 *
 * $VERSION$
 *
 * Author: Salva Peiró <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdarg.h>
#include <string.h>
#include <xm.h>

void *memset(void *s, xm_s32_t c, xm_u32_t n) {
    xm_u8_t* p=s;
    while(n--)
        *p++ = (xm_u8_t)c;
    return s;
}

xm_s32_t memcmp(const void* s1, const void* s2, xm_u32_t n) {
    const xm_u8_t *p1 = s1, *p2 = s2;
    while(n--)
        if (*p1 != *p2)
            return *p1 - *p2;
        else
            (void)*p1++, (void)*p2++;
    return 0;
}

char *strcpy(char *dest, const char* src) {
    xm_s8_t *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strncpy(char *dest, const char *src, xmSize_t n) {
	xm_s32_t j;

	memset(dest,0,n);

	for (j=0; j<n && src[j]; j++)
	dest[j]=src[j];

	if (j>=n)
	dest[n-1]=0;

	return dest;
}

xm_s32_t strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const xm_u8_t*)s1-*(const xm_u8_t*)s2;
}

xm_u32_t strlen(const char *s) {
    xm_u32_t i;
    for (i = 0; s && s[i] != '\0'; i++)
        ;
    return i;
}

char *strcat(char *dest, const char *src) {
    char *ret = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strncat(char *dest, const char *src, xm_u32_t n) {
    char *ret = dest;
    while (*dest)
        dest++;
    while (n--)
        if (!(*dest++ = *src++))
            return ret;
    *dest = 0;
    return ret;
}

xm_s32_t strncmp(const char* s1, const char* s2, xm_u32_t n) {
    while(n--)
        if(*s1++!=*s2++)
            return *(xm_u8_t*)(s1 - 1) - *(xm_u8_t*)(s2 - 1);
    return 0;
}

char *strchr(const char *s, int c) {
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

void *memcpy(void *dest, const void *src, xm_u32_t n) {
    xm_s8_t *dp = dest;
    const xm_s8_t *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

xm_s32_t atoi(const char* s) {
    long int v=0;
    int sign=1;
    unsigned base;

    while ( *s == ' '  ||  (unsigned int)(*s - 9) < 5u){
        s++;
    }

    switch (*s) {
        case '-':
            sign=-1;
        case '+':
            ++s;
    }
	
    base=10;
	if (*s == '0') {
        if (*(s+1) == 'x') { /* WARNING: assumes ascii. */
            s++;
            base = 16;
        }
        else {
            base = 8;
        }
        s++;
	}

    while ((unsigned int) (*s - '0') < base){
        v = v * base + *s - '0'; 
        ++s;
    }

    if (sign==-1){
        return -v;
    }
    return v;
}
