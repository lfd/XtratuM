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
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

xm_s32_t control = 1;
xm_u32_t excRet;

void DivideExceptionHandler(trapCtxt_t *ctxt)                                       /* XAL trap API */
{
    PRINT("#Divide Exception propagated, ignoring...\n");
    ctxt->eip = excRet;
}

void PartitionMain(void)
{
    volatile xm_s32_t val = 0;

    XM_idle_self();

    InstallTrapHandler(IA32_DIVIDE_ERROR, DivideExceptionHandler); /* Install timer handler */

    __asm__ __volatile__("mov $1f, %0\n\t" : "=r" (excRet):);

    PRINT("Dividing by zero...\n");

    val = 10 / val;

    __asm__ __volatile__("1:\n\t");

    PRINT("Halting\n");
    XM_halt_partition(XM_PARTITION_SELF);
}
