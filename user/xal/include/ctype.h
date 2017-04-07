/*
 * $FILE: ctype.h
 *
 * c types
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_CTYPE_H_
#define _XAL_CTYPE_H_

static inline xm_s32_t isdigit(xm_s32_t ch) {
    return (xm_u32_t)(ch - '0') < 10u;
}

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

#endif
