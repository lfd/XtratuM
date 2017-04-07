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

#define PRINT(...) do { \
        xmTime_t now; \
        XM_get_time(XM_HW_CLOCK, &now); \
        printf("[P%d][%lld] ", XM_PARTITION_SELF, now); \
        printf(__VA_ARGS__); \
} while (0)

void PartitionMain(void)
{
    xm_s32_t seq = 0;

    while (1) {
        PRINT("Run %d\n", seq++);
        XM_idle_self();
    }
    XM_halt_partition(XM_PARTITION_SELF);
}
