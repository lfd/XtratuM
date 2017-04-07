/*
 * $FILE: xm_def.h
 *
 * XM's ia32 hardware configuration
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _ARCH_XM_DEF_H_
#define _ARCH_XM_DEF_H_

#define XM_OFFSET (CONFIG_XM_OFFSET)

#define XM_HWIRQ_NR CONFIG_NO_HWIRQS

#ifdef _XM_KERNEL_

#define __ARCH_MEMCPY
#define XM_START_VMMAP (XM_OFFSET+CONFIG_XM_LOAD_ADDR+xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size)
#define XM_END_VMMAP XM_PGT_DIR_VADDR

#endif

#endif
