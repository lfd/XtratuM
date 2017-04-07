/*
 * $FILE: xm_def.h
 *
 * XM's ia32 hardware configuration
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _ARCH_XM_DEF_H_
#define _ARCH_XM_DEF_H_

#define XM_OFFSET (CONFIG_XM_OFFSET)

#define XM_HWIRQ_NR CONFIG_NO_HWIRQS

#ifdef _XM_KERNEL_

#define __ARCH_MEMCPY
#define XM_START_VMMAP (XM_OFFSET+8*1024*1024)
#define XM_END_VMMAP XM_PGT_DIR_VADDR

#endif

#endif
