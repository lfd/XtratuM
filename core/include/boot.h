/*
 * $FILE: boot.h
 *
 * Processor functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_BOOT_H_
#define _XM_BOOT_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <kthread.h>

#define __VBOOT __attribute__ ((__section__ (".vboot.text")))
#define __VBOOTDATA __attribute__ ((__section__ (".vboot.data")))

#endif
