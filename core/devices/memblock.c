/*
 * $FILE: memblock.c
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <boot.h>
#include <brk.h>
#include <kdevice.h>
#include <stdc.h>

#ifdef CONFIG_DEV_MEMBLOCK
static kDevice_t *memBlockTab;

static struct memBlockData {
    xm_s32_t pos;
    struct xmcMemBlock *cfg;
} *memBlockData;


static xm_s32_t ResetMemBlock(const kDevice_t *kDev) {
    memBlockData[kDev->subId].pos=0;    
    return 0;
}

static xm_s32_t ReadMemBlock(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {
    ASSERT(buffer);
    if ((memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos-len)<0)
	len=memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos;
    
    memcpy(buffer, (xm_u8_t *)(memBlockData[kDev->subId].cfg->startAddr+memBlockData[kDev->subId].pos), len);
    memBlockData[kDev->subId].pos+=len;
    
    return len;
}

static xm_s32_t WriteMemBlock(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {
    ASSERT(buffer);
    if ((memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos-len)<0)
	len=memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos;
    
    memcpy((xm_u8_t *)(memBlockData[kDev->subId].cfg->startAddr+memBlockData[kDev->subId].pos), buffer, len);
    memBlockData[kDev->subId].pos+=len;
    
    return len;
}

static xm_s32_t SeekMemBlock(const kDevice_t *kDev, xm_u32_t offset, xm_u32_t whence) {
    xm_s32_t off=offset;
    switch((whence)) {
    case DEV_SEEK_START:	
	break;
    case DEV_SEEK_CURRENT:
	off+=memBlockData[kDev->subId].pos;
	break;
    case DEV_SEEK_END:
	off+=memBlockData[kDev->subId].cfg->size;
	break;
    }
    if (off<0) off=0;
    if (off>memBlockData[kDev->subId].cfg->size) 
	off=memBlockData[kDev->subId].cfg->size;
    memBlockData[kDev->subId].pos=off;
    return off;
}

static xm_s32_t __VBOOT InitMemBlock(void) {
    xm_s32_t e;
    
    GET_MEMZ(memBlockTab, sizeof(kDevice_t)*xmcTab.deviceTab.noMemBlocks);
    GET_MEMZ(memBlockData, sizeof(struct memBlockData)*xmcTab.deviceTab.noMemBlocks);

    for (e=0; e<xmcTab.deviceTab.noMemBlocks; e++) {
	memBlockTab[e]=(kDevice_t) {
	    .subId=e,
	    .Reset=ResetMemBlock,
	    .Write=WriteMemBlock,
	    .Read=ReadMemBlock,
	    .Seek=SeekMemBlock,
	};
	memBlockData[e]=(struct memBlockData){
	    .pos=0,
	    .cfg=&xmcMemBlockTab[e],
	};
    }
    return 0;
}

//static void ShutdownMemBlock(void) {
//}

static const kDevice_t *GetMemBlock(xm_u32_t subId) {    
    return &memBlockTab[subId];
}

const struct kDevReg memBlockReg={
    .id=XM_DEV_LOGSTORAGE_ID,
    .Init=InitMemBlock,
    .Shutdown=0, //ShutdownMemBlock,
    .GetKDev=GetMemBlock,
};

REGISTER_KDEV_SETUP(memBlockReg);
#endif
