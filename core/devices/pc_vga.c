/*
 * $FILE: pc_vga.c
 *
 * ia32 PC screen driver
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <kdevice.h>
#ifdef CONFIG_DEV_PC_VGA
#include <assert.h>
#include <stdc.h>
#include <processor.h>
#include <arch/vga_text.h>
#include <kdevice.h>

#define VGA_COLUMNS 80
#define VGA_LINES 25
#define VGA_ATTR 7

#define BUFFER_SIZE (VGA_COLUMNS*VGA_LINES*2)

RESERVE_PHYSPAGES(0xFC0b8000, BUFFER_SIZE/PAGE_SIZE);

static xm_u8_t *buffer;
static xm_s32_t xPos, yPos;
static xm_s32_t init=0;

static xm_s32_t InitTextVga(void) {
    xm_s32_t pos;
  
    if (!init) {
	buffer=(xm_u8_t *)TEXT_VGA_ADDRESS;
	xPos=yPos=0;
	for (pos=0; pos<VGA_COLUMNS*VGA_LINES; pos++)
	    ((xm_u16_t *)buffer)[pos]=(VGA_ATTR << 8);
	
	VgaSetStartAddr(0);
	VgaSetCursorPos(0);
	init=1;	
    } 
    return 0;
}

static inline void PutCharTextVga(xm_s32_t c) {
    //xm_u32_t hwFlags;
    xm_s32_t pos;

    // HwSaveFlagsCli(hwFlags);
    if (c == '\t') {
	xPos += 3;
	if (xPos>=VGA_COLUMNS)
	    goto newline;
	VgaSetCursorPos((xPos+yPos*VGA_COLUMNS));
//	HwRestoreFlags(hwFlags);
	return;
    }

    if (c=='\n'||c=='\r') {
    newline:
	yPos++;
	xPos=0;
	if (yPos==VGA_LINES) {
	    memcpy((xm_u8_t *) buffer, (xm_u8_t *) &buffer [VGA_COLUMNS * 2], (VGA_LINES - 1) * VGA_COLUMNS * 2);
      
	    for (pos = 0; pos < VGA_COLUMNS; pos ++)
		((xm_u16_t *) buffer)[pos+(VGA_LINES-1)*VGA_COLUMNS]=(VGA_ATTR << 8);
	    yPos--;
	}
    
	VgaSetCursorPos((xPos+yPos*VGA_COLUMNS));
	//HwRestoreFlags(hwFlags);
	return;
    }
  
    buffer[(xPos+yPos*VGA_COLUMNS)*2]=c&0xFF;
    buffer[(xPos+yPos*VGA_COLUMNS)*2+1]=VGA_ATTR;

    xPos++;
    if (xPos>=VGA_COLUMNS)
	goto newline;
  
    VgaSetCursorPos(xPos+yPos*VGA_COLUMNS);
    //  HwRestoreFlags(hwFlags);
}

static xm_s32_t WriteTextVga(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {
    xm_s32_t e;
    for (e=0; e<len; e++)
	PutCharTextVga(buffer[e]);

    return len;
}

static const kDevice_t textVga={
    .subId=0,
    .Reset=0,
    .Write=WriteTextVga,
    .Read=0,
    .Seek=0,
};

static const kDevice_t *GetTextVga(xm_u32_t subId) {
    switch(subId) {
    case 0:
	return &textVga;
	break;
    }

    return 0;
}
const struct kDevReg textVgaReg={
    .id=XM_DEV_PC_VGA_ID,
    .Init=InitTextVga,
    .GetKDev=GetTextVga,
};

REGISTER_KDEV_SETUP(textVgaReg);

#ifdef CONFIG_EARLY_KOUTPUT
const kDevice_t *GetEarlyOutput(void) {
    InitTextVga();
    init=1;
    return &textVga;
}
#endif

#endif
