/*
 * $FILE: partition.c
 *
 * Fent Innovative Software Solutions
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <string.h>
#include <stdio.h>
#include <xm.h>
#include <irqs.h>
#include <config.h>

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

//xm_u8_t customFile[2048];                                         /* <-- BAD. Watch .bss zeroing */
__attribute__((section(".custom"))) xm_u8_t customFile[2048] = {0}; /* <-- OK  */

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
    .noModules = 1,
    .moduleTab={
        [0]={
            .sAddr=(xmAddress_t)customFile,
            .size=2048,
        },
    },
};

void PartitionMain(void)
{
    PRINT("----- Contents of the Custom File -----\n");
    PRINT("%s\n", customFile);
    PRINT("----- Contents of the Custom File -----\n");
    XM_halt_partition(XM_PARTITION_SELF);
}
