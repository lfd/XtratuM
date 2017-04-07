/*
 * $FILE: vga_text.h
 *
 * VGA text mode
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_VTEXT_H_
#define _XM_ARCH_VTEXT_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <arch/io.h>
#include <arch/xm_def.h>

#define TEXT_VGA_ADDRESS ((xm_u8_t*) (0xb8000 + XM_OFFSET))

#define R_MISC_OUTPUT 0x3cc
#define W_MISC_OUTPUT 0x3c2

#define CRTC_COM_REG 0x3d4
#define CRTC_DATA_REG 0x3d5
#define CURSOR_LOC_H 0x0e
#define CURSOR_LOC_L 0x0f
#define START_ADDR_H 0x0c
#define START_ADDR_L 0x0d

#define SEQ_COM_REG 0x3c4
#define SEQ_DATA_REG 0x3c5
#define CHAR_MAP_SELEC 0x3

#define VgaSetCursorPos(cursorPos) do { \
    OutBP(CURSOR_LOC_H, CRTC_COM_REG); \
    OutBP((xm_u8_t)(((cursorPos) >> 8)) & 0xff, CRTC_DATA_REG); \
    OutBP(CURSOR_LOC_L, CRTC_COM_REG); \
    OutBP((xm_u8_t)((cursorPos) & 0xff), CRTC_DATA_REG); \
} while(0)

#define VgaGetCursorPos() do { \
    xm_u8_t tmp_H = 0, tmp_L = 0; \
    xm_u16_t tmp; \
    OutBP(CURSOR_LOC_H, CRTC_COM_REG); \
    tmp_H = InBP(CRTC_DATA_REG); \
    OutBP(CURSOR_LOC_L, CRTC_COM_REG); \
    tmp_L = InBP(CRTC_DATA_REG); \
    tmp = (xm_u16_t) ((tmp_H << 8) | tmp_L); \
    tmp; \
} while(0)

#define VgaSetStartAddr(startAddr) do { \
    OutBP(START_ADDR_H, CRTC_COM_REG); \
    OutBP((xm_u8_t)(((startAddr)>>8)) & 0xff, CRTC_DATA_REG);	\
    OutBP(START_ADDR_L, CRTC_COM_REG); \
    OutBP((xm_u8_t)((startAddr)&0xff), CRTC_DATA_REG);	\
} while(0)

#define VgaGetStartAddr() do { \
    xm_u8_t tmp_H = 0, tmp_L = 0; \
    xm_u16_t tmp; \
    OutBP(START_ADDR_H, CRTC_COM_REG); \
    tmp_H = InBP(CRTC_DATA_REG); \
    OutBP(START_ADDR_L, CRTC_COM_REG); \
    tmp_L = InBP(CRTC_DATA_REG); \
    tmp = (xm_u16_t) ((tmp_H << 8) | tmp_L); \
    tmp; \
} while(0)

#define VgaSetCharMapSelect() do { \
    xm_u8_t tmp = 0; \
    OutBP(CHAR_MAP_SELEC, SEQ_COM_REG); \
    tmp = (xm_u8_t) InBP(SEQ_DATA_REG); \
    tmp; \
} while(0)

#endif // _ARCH_VTEXT_H_
