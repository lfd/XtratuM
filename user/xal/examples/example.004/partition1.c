/*
 * $FILE: partition1.c
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

#define QPORT_NAME      "portQ"
#define SPORT_NAME      "portS"
#define SHARED_ADDRESS  0x2300000

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

char sMessage[32];
char qMessage[32];
xm_s32_t qDesc, sDesc, seq;

void ChannelExtHandler(trapCtxt_t *ctxt)
{
    xm_u32_t flags;

    if (XM_receive_queuing_message(qDesc, qMessage, sizeof(qMessage), &flags) > 0) {
        PRINT("RECEIVE %s\n", qMessage);
        PRINT("SHM WRITE %d\n", seq);
        *(volatile xm_u32_t *)SHARED_ADDRESS = seq++;
    }

    if (XM_read_sampling_message(sDesc, sMessage, sizeof(sMessage), &flags) > 0) {
        PRINT("RECEIVE %s\n", sMessage);
    }
    XM_unmask_irq(XM_VT_EXT_OBJDESC);
}

void PartitionMain(void)
{
    PRINT("Opening ports...\n");
    qDesc = XM_create_queuing_port(QPORT_NAME, 16, 128, XM_DESTINATION_PORT);
    if (qDesc < 0) {
        PRINT("error %d\n", qDesc);
        goto end;
    }
    sDesc = XM_create_sampling_port(SPORT_NAME, 128, XM_DESTINATION_PORT);
    if (sDesc < 0) {
        PRINT("error %d\n", sDesc);
        goto end;
    }
    PRINT("done\n");

    InstallTrapHandler(XAL_XMEXT_TRAP(XM_VT_EXT_OBJDESC), ChannelExtHandler);
    HwSti();
    XM_unmask_irq(XM_VT_EXT_OBJDESC);

    PRINT("Waiting for messages\n");
    while (1);

end:
    XM_halt_partition(XM_PARTITION_SELF);
}
