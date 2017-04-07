/*
 * $FILE: constraints.h
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XMC_CONSTRAINTS_H_
#define _XMC_CONSTRAINTS_H_

extern void AddConstraint(char *name, int (*Checker)(struct xmc *xmcTab));
extern void SetLastMemoryAreaShared(void);
extern void CheckConstraints(struct xmc *xmcTab);
extern void AddMemoryArea(unsigned long start, unsigned long end, int stLine, int endLine);
extern void AddMemoryRegion(unsigned long start, unsigned long end, int stLine, int endLine);
extern int IsPartition(int partition);
extern void AddPartition(int partition);
extern void InitConstraints(void);

#endif
