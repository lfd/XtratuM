/*
 * $FILE: config.h
 *
 * Config file
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_CONFIG_H_
#define _XM_CONFIG_H_

#ifdef _XM_KERNEL_
#include <autoconf.h>
#else
#include <xm_inc/autoconf.h>
#endif

// bits: (31..24)(23..16)(15..8)(7..0)
// Reserved.VERSION.SUBVERSION.REVISION
#define XM_VERSION (((CONFIG_VERSION&0xFF)<<16)|((CONFIG_SUBVERSION&0xFF)<<8)|(CONFIG_REVISION&0xFF))

#define CONFIG_KSTACK_SIZE (CONFIG_KSTACK_KB*1024)

#if (CONFIG_ID_STRING_LENGTH&3)
#error CONFIG_ID_STRING_LENGTH must be a power of 4
#endif

#endif
