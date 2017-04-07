/*
 * $FILE: xm-linux.h
 *
 * Lib XM Linux (user version)
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_LINUX_H_
#define _XM_LINUX_H_

#include <xm_inc/arch/arch_types.h>
#include <xm.h>

#define XMDEV	"/dev/xmctl"

typedef struct LibXmInfo LibXmInfo;
struct LibXmInfo {
    int fd;
    unsigned int cmd;
    unsigned int arg[5];
};

xm_s32_t XM_hcall(int cmd, xm_u32_t a0, xm_u32_t a1, xm_u32_t a2, xm_u32_t a3, xm_u32_t a4);

#undef XM_PARTITION_SELF
#define XM_PARTITION_SELF (XM_hcall(NR_HYPERCALLS, 0, 0, 0, 0, 0))

#endif // _XM_LINUX_H_
