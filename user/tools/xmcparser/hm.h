/*
 * $FILE: hm.h
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XMC_HM_H_
#define _XMC_HM_H_
#include <xm_inc/arch/arch_types.h>
#include <xm_inc/xmconf.h>

#define MAX_HM_ACTIONS 7
extern char *hmEvents[];
extern char *hmActions[];
extern char *hmLog[];
extern int hmHpvEventsAndActions[XM_HM_MAX_EVENTS];
extern int hmPartEventsAndActions[XM_HM_MAX_EVENTS];

extern void InitHm(void);
extern void SetDefPartHmTab(struct xmcHmSlot *hmTab);
extern void SetDefHpvHmTab(struct xmcHmSlot *hmTab);

#endif
