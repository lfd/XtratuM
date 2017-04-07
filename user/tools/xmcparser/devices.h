/*
 * $FILE: devices.h
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XMC_DEVICES_H_
#define _XMC_DEVICES_H_

extern void InsertDevice(char *name, xmDev_t dev);
extern void LookUpDevice(char *name, xmDev_t *dev);
extern int InsertDevAsoc(char *name);
extern char *GetDevAsocName(unsigned int id);

#ifdef __x86_64__
#define REGISTER_DEVICE(_init) REGISTER_DEVICE64(_init)
#else
#define REGISTER_DEVICE(_init) REGISTER_DEVICE32(_init)
#endif

#define REGISTER_DEVICE32(_init) \
  __asm__ (".section .devtab, \"ax\"\n\t" \
           ".align 4\n\t" \
           ".long "#_init"\n\t" \
           ".previous\n\t")

#define REGISTER_DEVICE64(_init) \
  __asm__ (".section .devtab, \"a\"\n\t" \
           ".align 8\n\t" \
           ".quad "#_init"\n\t" \
           ".previous\n\t")

#endif
