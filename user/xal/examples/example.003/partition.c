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

#define TRACE_MASK  0x3

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

void PartitionMain(void)
{
    xmTraceEvent_t event;

    event.opCode.moduleId=XM_PARTITION_SELF;
    event.opCode.criticality=XM_TRACE_WARNING;
    if (XM_PARTITION_SELF == 0) {
        event.word[0]=0x0;
    } else {
        event.word[0]=0xFFFF;
    }

    while (1) {
        PRINT("New trace: %x\n", event.word[0]);
        XM_trace_event(0x01, &event);
        if (XM_PARTITION_SELF == 0) {
            event.word[0]++;
        } else {
            event.word[0]--;
        }
        XM_idle_self();
    }
    XM_halt_partition(XM_PARTITION_SELF);
}
