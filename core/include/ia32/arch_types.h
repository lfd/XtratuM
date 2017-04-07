/*
 * $FILE: arch_types.h
 *
 * Types defined by the architecture
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_TYPES_H_
#define _XM_ARCH_TYPES_H_

/*  <track id="ia32-basic-types"> */
// Basic types
typedef unsigned char xm_u8_t;
typedef char xm_s8_t;
typedef unsigned short xm_u16_t;
typedef short xm_s16_t;
typedef unsigned int xm_u32_t;
typedef int xm_s32_t;
typedef unsigned long long xm_u64_t;
typedef long long xm_s64_t;
/*  </track id="ia32-basic-types"> */

/*  <track id="ia32-extended-types"> */
// Extended types
typedef xm_s64_t xmTime_t;
typedef xm_u32_t xmAddress_t;
typedef xm_u16_t xmIoAddress_t;
typedef xm_u32_t xmSize_t;
typedef xm_s32_t xmSSize_t;
typedef xm_u32_t xmId_t;
/*  </track id="ia32-extended-types"> */

#ifdef _XM_KERNEL_

// Extended internal types
typedef xm_s64_t hwTime_t;

#endif
#endif
