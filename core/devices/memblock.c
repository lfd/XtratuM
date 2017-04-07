/*
 * $FILE: memblock.c
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <boot.h>
#include <brk.h>
#include <kdevice.h>
#include <stdc.h>
#include <virtmm.h>
#include <vmmap.h>
#include <physmm.h>

#ifdef CONFIG_DEV_MEMBLOCK
static kDevice_t *memBlockTab;

extern void *MemcpyPhys(void *dst, const void *src, xm_u32_t count);

static struct memBlockData {
    xm_s32_t pos;
    struct xmcMemBlock *cfg;
    xmAddress_t addr;
} *memBlockData;


static xm_s32_t ResetMemBlock(const kDevice_t *kDev) {
    memBlockData[kDev->subId].pos=0;    
    return 0;
}

static xm_s32_t ReadMemBlock(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {
    ASSERT(buffer);
    if ((memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos-len)<0)
	len=memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos;
    
    memcpy(buffer, (xm_u8_t *)(memBlockData[kDev->subId].addr+memBlockData[kDev->subId].pos), len);
    //MemcpyPhys(buffer, (xm_u8_t *)(memBlockData[kDev->subId].cfg->startAddr+memBlockData[kDev->subId].pos), len);
    memBlockData[kDev->subId].pos+=len;
    
    return len;
}

static xm_s32_t WriteMemBlock(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {
    ASSERT(buffer);
    if ((memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos-len)<0)
	len=memBlockData[kDev->subId].cfg->size-memBlockData[kDev->subId].pos;
    
    memcpy((xm_u8_t *)(memBlockData[kDev->subId].addr+memBlockData[kDev->subId].pos), buffer, len);
    //MemcpyPhys((xm_u8_t *)(memBlockData[kDev->subId].cfg->startAddr+memBlockData[kDev->subId].pos), buffer, len);
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
    xm_s32_t i, noPages;

    GET_MEMZ(memBlockTab, sizeof(kDevice_t)*xmcTab.deviceTab.noMemBlocks);
    GET_MEMZ(memBlockData, sizeof(struct memBlockData)*xmcTab.deviceTab.noMemBlocks);

    for (e = 0; e < xmcTab.deviceTab.noMemBlocks; e++) {
        memBlockTab[e] = (kDevice_t) {
                    .subId=e,
                    .Reset=ResetMemBlock,
                    .Write=WriteMemBlock,
                    .Read=ReadMemBlock,
                    .Seek=SeekMemBlock,
                };
        memBlockData[e]=(struct memBlockData) {
            .pos=0,
            .cfg=&xmcMemBlockTab[e],
        };
        noPages=SIZE2PAGES(xmcMemBlockTab[e].size);
        if (!(memBlockData[e].addr=VmmAlloc(noPages))) {
            SystemPanic(0, 0, "[InitMemBlock] System is out of free frames\n");
        }
        for (i=0; i<(noPages*PAGE_SIZE); i+=PAGE_SIZE)
            VmMapPage(xmcMemBlockTab[e].startAddr+i, memBlockData[e].addr+i,_PG_PRESENT|_PG_RW);
        kprintf("Mapped memory block device at 0x%x\n", memBlockData[e].addr);
    }
    return 0;
}

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
