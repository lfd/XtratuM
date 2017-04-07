/*
 * $FILE: hypervisor.c
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

#include "xm-linux.h"
#include <xm_inc/objdir.h>
#include <xm_inc/objects/mem.h>

__stdcall xm_s32_t XM_write_console(char *buffer, xm_s32_t length) {
    return XM_write_object(OBJDESC_BUILD(OBJ_CLASS_CONSOLE, XM_PARTITION_SELF, 0), buffer, length, 0);
}

__stdcall xm_s32_t XM_memory_copy(xmId_t dstId, xm_u32_t dstAddr, xmId_t srcId, xm_u32_t srcAddr, xm_u32_t size) {
    if (srcId == XM_PARTITION_SELF)
        return XM_write_object(OBJDESC_BUILD(OBJ_CLASS_MEM, XM_PARTITION_SELF, 0), (void *) dstAddr, size, (void *) srcAddr);
    if (dstId == XM_PARTITION_SELF)
        return XM_write_object(OBJDESC_BUILD(OBJ_CLASS_MEM, XM_PARTITION_SELF, 0), (void *) srcAddr, size, (void *) dstAddr);
    return XM_INVALID_PARAM;
}

__stdcall xm_s32_t XM_get_physmem_map(struct xmcMemoryArea *memMap, xm_s32_t noAreas) {
    union memCmd args;
    
    args.physMemMap.areas=memMap;
    args.physMemMap.noAreas=noAreas;
    
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_MEM, XM_PARTITION_SELF, 0), XM_MEM_GET_PHYSMEMMAP, &args);
}
