/*
 * FILE: main.c
 */

#include <string.h>
#include <stdio.h>
#include <xm.h>
#include "function.h"

void PartitionMain(void)
{
    xm_u32_t i;
    
    i = 0;
    while (1) {
        printf("[P%d] Function returns: %d\n", XM_PARTITION_SELF, func(i));
        i++;
        XM_idle_self();
    }
    
    XM_halt_partition(XM_PARTITION_SELF);
}
