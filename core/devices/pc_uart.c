/*
 * $FILE: pc_uart.c
 *
 * PC UART
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <kdevice.h>
#ifdef CONFIG_DEV_PC_UART
#include <arch/io.h>
#include <arch/uart.h>

RESERVE_HWIRQ(UART_IRQ0);
RESERVE_IOPORTS(DEFAULT_PORT, 5);

static xm_s32_t InitUart(void) {
    OutB(0x00, DEFAULT_PORT+1);    // Disable all interrupts
    OutB(0x80, DEFAULT_PORT+3);    // Enable DLAB (set baud rate divisor)
    //OutB(0x03, DEFAULT_PORT+0);    // Set divisor to 3 (lo byte) 38400 baud
    OutB(0x01, DEFAULT_PORT+0);    // Set divisor to 1 (lo byte) 115200 baud
    OutB(0x00, DEFAULT_PORT+1);    //                  (hi byte)
    OutB(0x03, DEFAULT_PORT+3);    // 8 bits, no parity, one stop bit
    OutB(0xC7, DEFAULT_PORT+2);    // Enable FIFO, clear them, with 14-byte threshold
    OutB(0x0B, DEFAULT_PORT+4);    // IRQs enabled, RTS/DSR set
    return 0;
}

static inline void PutCharUart(xm_s32_t c) {
    while (!(InB(DEFAULT_PORT+5)&0x20))
        continue;
    OutB(c, DEFAULT_PORT);
}

static xm_s32_t WriteUart(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {
    xm_s32_t e;
    for (e=0; e<len; e++)
	PutCharUart(buffer[e]);
    
    return len;
}

static const kDevice_t uartDev={
    .subId=0,
    .Reset=0,
    .Write=WriteUart,
    .Read=0,
    .Seek=0,
};

static const kDevice_t *GetUart(xm_u32_t subId) {
    switch(subId) {
    case 0:
        return &uartDev;
        break;
    }

    return 0;
}

const struct kDevReg uartReg={
    .id=XM_DEV_PC_UART_ID,
    .Init=InitUart,
    .GetKDev=GetUart,
};

REGISTER_KDEV_SETUP(uartReg);

#endif
