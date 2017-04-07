/*
 * $FILE: hdr.c
 *
 * XtratuM Partition Image Header 
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 * Modified: Salva Peiró <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <xm_inc/arch/asm_offsets.h>

extern xm_u32_t _sguest[];
extern xm_u32_t _eguest[];
extern struct xmPartitionHdr __xmPartitionHdr[];

struct xmImageHdr __xmImageHdr __attribute__ ((section(".text.init #alloc,#exclude"))) __attribute__((weak)) = {
    .signature=XMEF_PARTITION_MAGIC,
    .xmAbiVersion=XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION),
    .xmApiVersion=XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION),
    .imageId = 0,
    .checksum = 0,
    .sAddr = (xmAddress_t) _sguest,
    .eAddr = (xmAddress_t) _eguest,
    .entry = {
        .defaultPartitionHdr = __xmPartitionHdr,
    },
    .noModules = 0,
    .moduleTab={
        [0]={
            .sAddr=(xmAddress_t)0,
            .size=0,
        },
    },
};
