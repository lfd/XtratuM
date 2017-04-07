/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_IRQS_H_
#define _XAL_IRQS_H_

#include <arch/irqs.h>
#include <xm.h>

#define XAL_XMEXT_TRAP(_xmtrap)  ((( _xmtrap ) - XM_VT_EXT_FIRST)+EXT_IRQ_VECTOR)

typedef void (*trapHandler_t)(trapCtxt_t *);

extern trapHandler_t trapHandlersTab[];
extern xm_s32_t InstallTrapHandler(xm_s32_t trapNr, trapHandler_t handler);
//extern void SetupIrqs(void);

#endif
