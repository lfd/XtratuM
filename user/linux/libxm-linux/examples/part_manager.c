/*
 * $FILE: part_manager.c
 *
 * Simple partition manager example using LibXM Linux (user version)
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xm-linux.h"

int main(int argc, char *argv[])
{
    int partitionId;
    int i;
    int delay;

    if (argc < 2) {
        printf("usage: part_manager partitionId\n");
        return -1;
    }

    init_libxm(0,0);

    partitionId = atoi(argv[1]);
    if (partitionId == 0) {
        printf("part_manager: error: invalid partitionId\n");
        return -1;
    }
    printf("Partition manager running on partition %d\n", XM_PARTITION_SELF);

    delay = 10;
    printf("waiting %d seconds to suspend partition %d:", delay, partitionId);
    for (i = 0; i < delay; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    XM_suspend_partition(partitionId);
    printf(" done\n");

    delay = 10;
    printf("waiting %d seconds to resume partition %d:", delay, partitionId);
    for (i = 0; i < delay; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    XM_resume_partition(partitionId);
    printf(" done\n");

    delay = 20;
    printf("waiting %d seconds to halt partition %d:", delay, partitionId);
    for (i = 0; i < delay; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    XM_halt_partition(partitionId);
    printf(" done\n");

    delay = 20;
    printf("waiting %d seconds to reset partition %d:", delay, partitionId);
    for (i = 0; i < delay; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }

    XM_reset_partition(partitionId, XM_COLD_RESET, 0);
    printf(" done\n");

    return 0;
}
