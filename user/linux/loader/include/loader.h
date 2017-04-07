/*
 * $FILE: loader.h
 *
 * LinuxLoader
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _LINUX_LOADER_H_
#define _LINUX_LOADER_H_

#include <xm.h>

extern partitionControlTable_t partitionControlTable;
extern partitionInformationTable_t partitionInformationTable;

extern void JumpToLinux(xmAddress_t ePoint);
extern void SetupVMMap(xmAddress_t offset);
//extern xmAddress_t LowestAddress(void);
extern char _slinuxImg[], _slinuxloader[];
extern xmAddress_t _pgdAddr[];

#endif
