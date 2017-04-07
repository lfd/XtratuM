/*
 * $FILE: assert.h
 *
 * Assert definition
 *
 * $VERSION$
 * 
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ASSERT_H_
#define _XM_ASSERT_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <processor.h>

#ifdef CONFIG_DEBUG

#define ASSERT(exp)  \
    ((exp)?0:SystemPanic(0, 0, __FILE__":%u: failed assertion `"#exp"'\n", __LINE__))

#else

#define ASSERT(exp) ((void)0)

#endif

#endif
