/*
 * $FILE: supervisor.c
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

#define P0 0
#define P1 1
#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

static inline int GetStatus(xmPartitionStatus_t *partStatus) {
    return (partStatus->state);
}

static char *statusString[] = {
        [XM_STATUS_HALTED] = "halted",
        [XM_STATUS_IDLE] = "idle",
        [XM_STATUS_READY] = "ready",
        [XM_STATUS_SUSPENDED] = "suspended",
};

void PrintStatus(void) {
    static xmPartitionStatus_t partStatus;
    int st1, st2;

    XM_partition_get_status(P0, &partStatus);
    st1 = GetStatus(&partStatus);

    XM_partition_get_status(P1, &partStatus);
    st2 = GetStatus(&partStatus);
    PRINT("Status P0 => %s ; P1 => %s\n", statusString[st1], statusString[st2]);
}

void PartitionMain(void) {
    int retValue;

    PrintStatus();
    XM_idle_self();
    retValue = XM_suspend_partition(P0);
    PRINT("Suspend P%d, %d\n", P0, retValue);
    if (retValue >= 0)
        PrintStatus();
    XM_idle_self();
    retValue = XM_suspend_partition(P1);
    PRINT("Suspend P%d, %d\n", P1, retValue);
    if (retValue >= 0)
        PrintStatus();

    XM_idle_self();

    retValue = XM_resume_partition(P0);
    PRINT("Resume P%d, %d\n", P0, retValue);
    if (retValue >= 0)
        PrintStatus();
    XM_idle_self();
    retValue = XM_resume_partition(P1);
    PRINT("Resume P%d, %d\n", P1, retValue);
    if (retValue >= 0)
        PrintStatus();
    XM_idle_self();

    retValue = XM_halt_partition(P0);
    PRINT("Halt P%d, %d\n", P0, retValue);
    if (retValue >= 0)
        PrintStatus();

    XM_idle_self();
    retValue = XM_halt_partition(P1);
    PRINT("Halt P%d, %d\n", P1, retValue);
    if (retValue >= 0)
        PrintStatus();
    XM_idle_self();

    retValue = XM_reset_partition(P0, XM_WARM_RESET, 0);
    PRINT("Restart P%d, %d\n", P0, retValue);
    if (retValue >= 0)
        PrintStatus();
    XM_idle_self();
    retValue = XM_reset_partition(P1, XM_WARM_RESET, 0);
    PRINT("Restart P%d, %d\n", P1, retValue);
    if (retValue >= 0)
        PrintStatus();
    XM_idle_self();

    PRINT("Halting System ...\n", XM_PARTITION_SELF);
    retValue = XM_halt_system();

}

