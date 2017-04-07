/*
 * $FILE: linux_boot.c
 *
 * LinuxLoader
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <asm/bootparam.h>
#include <xm_inc/xmef.h>

static struct boot_params boot_params={
    .hdr={
		.version=0x207,
		.hardware_subarch=3,
		.loadflags=KEEP_SEGMENTS,
		.type_of_loader=0xa0,
    },
    .screen_info={
		.orig_x = 0, /* 0x00 */
		.orig_y = 0, /* 0x01 */
		.orig_video_page = 8, /* 0x04 */
		.orig_video_mode = 3, /* 0x06 */
		.orig_video_cols = 80, /* 0x07 */
		.orig_video_ega_bx = 3, /* 0x0a */
		.orig_video_lines = 25, /* 0x0e */
		.orig_video_isVGA = 1, /* 0x0f */
		.orig_video_points = 16, /* 0x10 */
    },
    .e820_entries=0,
};

void JumpToLinux(xmAddress_t ePoint) {
    extern struct xmPartitionHdr __xmPartitionHdr[];
    extern struct xmImageHdr __xmImageHdr[];
    xm_s32_t e;
    char *c;
    boot_params.hdr.hardware_subarch_data=(__u32)__xmPartitionHdr;
    boot_params.hdr.cmd_line_ptr=__xmImageHdr[0].moduleTab[0].sAddr;
    boot_params.hdr.cmdline_size=__xmImageHdr[0].moduleTab[0].size;
    if (__xmImageHdr[0].moduleTab[1].size) {
	boot_params.hdr.ramdisk_image=__xmImageHdr[0].moduleTab[1].sAddr;
	boot_params.hdr.ramdisk_size=__xmImageHdr[0].moduleTab[1].size;
    }
    c=(char *)boot_params.hdr.cmd_line_ptr;
    for (e=0; e<boot_params.hdr.cmdline_size; e++, c++)
	if ((*c==10)||(*c==13))
	    *c=0;
    boot_params.hdr.setup_data = __xmImageHdr[0].moduleTab[2].sAddr;
    
    __asm__ __volatile__ ("jmp *%1\n\t" :: "S" (&boot_params) , "m" (ePoint));
}
