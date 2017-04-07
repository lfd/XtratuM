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
#include <string.h>
#include <stdio.h>
#include <xm.h>
#include <irqs.h>

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

static inline void readTrace(xm_s32_t pid)
{
    xm_s32_t tid;
    xmTraceEvent_t event;

    tid = XM_trace_open(pid);
    if (XM_trace_read(tid, &event) > 0) {
        PRINT("[Trace] partitionId: %x, moduleId: %x, criticality: %x, code: %x, timeStamp: %lld, payload: %x\n",
                event.opCode.partitionId, event.opCode.moduleId, event.opCode.criticality, event.opCode.code,
                event.timeStamp, event.word[0]);
    }
}

void PartitionMain(void)
{
    PRINT(" --------- Trace Log ---------------\n");
    while (1) {
        readTrace(0);
        readTrace(1);
        XM_idle_self();
    }

    XM_halt_partition(XM_PARTITION_SELF);
}
