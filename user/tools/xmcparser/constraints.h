/*
 * $FILE: constraints.h
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
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
