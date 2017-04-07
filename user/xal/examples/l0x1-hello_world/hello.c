/*
 * $FILE: hello.c
 *
 * hello example
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <string.h>
#include <stdio.h>
#include <xm.h>

void PartitionMain(void) {
    XM_write_console("Hello World!\n", 14);
//    XM_halt_system();
    XM_halt_partition(XM_PARTITION_SELF);
}
