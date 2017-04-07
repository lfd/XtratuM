/*
 * $FILE: hello.c
 *
 * hello example
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <string.h>
#include <stdio.h>
#include <xm.h>

void PartitionMain(void) {
    XM_write_console("Hello World!\n", 14);
    XM_halt_partition(XM_PARTITION_SELF);
}
