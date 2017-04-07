/*
 * $FILE: console.h
 *
 * Console definition
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
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
