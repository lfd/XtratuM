/*
 * $FILE: console.h
 *
 * Console definition
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJ_CONSOLE_H_
#define _XM_OBJ_CONSOLE_H_

#ifdef _XM_KERNEL_
#include <kdevice.h>

extern void ConsoleInit(const kDevice_t *kDev);
extern void ConsolePutChar(char c);
#endif
#endif
