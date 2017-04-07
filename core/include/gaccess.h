/*
 * $FILE: gaccess.h
 *
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_GACCESS_H_
#define _XM_GACCESS_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <arch/gaccess.h>

#define __gParam __archGParam

#define __CheckGParam(__ctxt, __param, __size) ({ \
    xm_s32_t __r=__ArchCheckGParam((__ctxt), (__param), (__size)); \
    __r; \
})

#endif
