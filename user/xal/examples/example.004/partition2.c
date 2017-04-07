/*
 * $FILE: partition2.c
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

#define SPORT_NAME "portS"
#define SHARED_ADDRESS  0x2300000

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

char sMessage[32];
xm_s32_t sDesc;
xm_u32_t flags;

void PartitionMain(void)
{
    PRINT("Opening ports...\n");                                            /* Create ports */
    sDesc = XM_create_sampling_port(SPORT_NAME, 128, XM_DESTINATION_PORT);
    if (sDesc < 0) {
        PRINT("error %d\n", sDesc);
        goto end;
    }
    PRINT("done\n");

    PRINT("Waiting for messages\n");
    while (1) {
        if (XM_read_sampling_message(sDesc, sMessage, sizeof(sMessage), &flags) > 0) {
            PRINT("RECEIVE %s\n", sMessage);
            PRINT("READ SHM %d\n", *(volatile xm_u32_t *)SHARED_ADDRESS);
        }
        XM_idle_self();
    }

end:
    XM_halt_partition(XM_PARTITION_SELF);
}
