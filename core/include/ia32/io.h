/*
 * $FILE: io.h
 *
 * Port's related functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_IO_H_
#define _XM_ARCH_IO_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#define IoDelay() \
    __asm__ __volatile__ ("pushl %eax; inb $0x80,%al; inb $0x80,%al; popl %eax")

#define OutB(val, port) \
    __asm__ __volatile__ ("outb %0, %%dx\n\t" \
                          :: "a" ((xm_u8_t) (val)), \
                          "d" ((xm_u16_t) (port)))

#define OutW(val, port) \
    __asm__ __volatile__ ("outw %0, %%dx\n\t" \
                          :: "a" ((xm_u16_t) (val)), \
                          "d" ((xm_u16_t) (port)))

#define OutL(val, port) \
    __asm__ __volatile__ ("outl %0, %%dx\n\t" \
                          :: "a" ((xm_u32_t) (val)), \
                          "d" ((xm_u16_t) (port)))

#define OutBP(val, port) ({ \
    OutB(val, port); \
    IoDelay(); \
    IoDelay(); \
    IoDelay(); \
})

#define InB(port) ({ \
    xm_u8_t __inb_port; \
    __asm__ __volatile__ ("inb %%dx, %0\n\t" : "=a" (__inb_port) \
                          : "d" ((xm_u16_t) (port))); \
    __inb_port; \
})

#define InW(port) ({ \
    xm_u16_t __inw_port; \
    __asm__ __volatile__ ("inw %%dx, %0\n\t" : "=a" (__inw_port) \
                          : "d" ((xm_u16_t) (port))); \
    __inw_port; \
})

#define InL(port) ({ \
    xm_u32_t __inl_port; \
    __asm__ __volatile__ ("inl %%dx, %0\n\t" : "=a" (__inl_port) \
                          : "d" ((xm_u16_t) (port))); \
    __inl_port; \
})

#define InBP(port) ({ \
    xm_u8_t __inb_port; \
    __inb_port = InB(port); \
    IoDelay(); \
    IoDelay(); \
    IoDelay(); \
    __inb_port; \
})

#endif
