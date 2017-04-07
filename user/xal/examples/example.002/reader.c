/*
 * $FILE: reader.c
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
#include <stdio.h>
#include <xm.h>

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

#define HALTED 3

static void PrintHmLog(xmHmLog_t *h) {
    PRINT("Log => partitionId: %d eventId: %d timeStamp: %lld\n", h->partitionId, h->eventId, h->timeStamp);
}

void PartitionMain(void) {
    xm_s32_t ret;
    xmPartitionStatus_t partStatus;
    xmHmStatus_t hmStatus;
    xmHmLog_t hmLog;

    XM_idle_self();

    PRINT(" --------- Health Monitor Log ---------------\n");
    while (1) {
        XM_hm_status(&hmStatus);
        if (hmStatus.currentEvent < hmStatus.noEvents) {
            ret=XM_hm_read(&hmLog);
            if (ret < 0) {
                PRINT("read hm log: FAILED %d\n", ret);
            }
            PrintHmLog(&hmLog);
        } else {
            XM_idle_self();
        }
    }
    PRINT("--------- Health Monitor Log ---------------\n");
}

