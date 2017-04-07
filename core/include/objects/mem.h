/*
 * $FILE: mem.h
 *
 * memory object definitions
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJ_MEM_H_
#define _XM_OBJ_MEM_H_

#define XM_MEM_GET_PHYSMEMMAP 0x1

union memCmd {
    struct getPhysMemMap {
	struct xmcMemoryArea *areas;
	xm_s32_t noAreas;
    } physMemMap;
};

#endif
