/*
 * $FILE: uart.c
 *
 * UART driver
 *
 * $VERSION$
 *
 * Author: S. Peiró, <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifdef CONFIG_OUTPUT_ENABLED

static inline void outb(xm_u8_t v, xm_u16_t port) {
    asm volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

static inline xm_u8_t inb(xm_u16_t port) {
    xm_u8_t v;
    asm volatile("inb %1,%0" : "=a" (v) : "dN" (port));
    return v;
}

#define SPORT0          0x3F8
#define SPORT1          0x2F8
#define SPORT2          0x3E8
#define SPORT3          0x2E8

#define DEFAULT_PORT    SPORT0

void xputchar(xm_s32_t c) {
    while (!(inb(DEFAULT_PORT+5)&0x20))
        continue;
    outb(c, DEFAULT_PORT);
}

static int UartWrite(char *s, int n) {
    xm_s32_t i;

    for (i=0; i<n; ++i) {
        xputchar(s[i]);
    }

    return n;
}

char UartReadChar(void) {
    xm_s32_t c;

    while (!(inb(DEFAULT_PORT+5)&0x01)) {
        continue;
    }

    c = inb(DEFAULT_PORT);

    return c;
}

void InitOutput(void)
{
    outb(0x00, DEFAULT_PORT+1);    // Disable all interrupts
    outb(0x80, DEFAULT_PORT+3);    // Enable DLAB (set baud rate divisor)
    outb(0x01, DEFAULT_PORT+0);    // Set divisor to 3 (lo byte) 115200 baud
    outb(0x00, DEFAULT_PORT+1);    //                  (hi byte)
    outb(0x03, DEFAULT_PORT+3);    // 8 bits, no parity, one stop bit
    outb(0xC7, DEFAULT_PORT+2);    // Enable FIFO, clear them, with 14-byte threshold
    outb(0x0B, DEFAULT_PORT+4);    // IRQs enabled, RTS/DSR set

    UartWrite("[RSW] Initialized UART\n", sizeof("[RSW] Initialized UART\n"));
}

#endif
