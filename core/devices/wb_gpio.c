/*
 * $FILE: wb_gpio.c
 *
 * Winbond GPIO driver
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <kdevice.h>
#ifdef CONFIG_DEV_WB_GPIO
#include <arch/io.h>

#define WB_GPIO_INDEX_PORT    0x2E
#define WB_GPIO_DATA_PORT     0x2F

#define LDREG   0x07

RESERVE_IOPORTS(WB_GPIO_INDEX_PORT, 2);

static inline void WbWrite(xm_s32_t reg, xm_s32_t val) {
    OutB(0x87, WB_GPIO_INDEX_PORT);     /* Step 1: Enter Extended Function */
    OutB(0x87, WB_GPIO_INDEX_PORT);
    OutB(reg, WB_GPIO_INDEX_PORT);      /* Step 2: Select register and write data */
    OutB(val, WB_GPIO_DATA_PORT);
    OutB(0xAA, WB_GPIO_INDEX_PORT);     /* Step 3: Exit Extended Function */
}

static inline xm_s32_t WbRead(xm_s32_t reg) {
    xm_s32_t val;

    OutB(0x87, WB_GPIO_INDEX_PORT);     /* Step 1: Enter Extended Function */
    OutB(0x87, WB_GPIO_INDEX_PORT);
    OutB(reg, WB_GPIO_INDEX_PORT);      /* Step 2: Select register and write data */
    val = InB(WB_GPIO_DATA_PORT);
    OutB(0xAA, WB_GPIO_INDEX_PORT);     /* Step 3: Exit Extended Function */

    return val;
}

static inline void WbWriteLdev(xm_s32_t ldev, xm_s32_t reg, xm_s32_t val) {
    WbWrite(0x07, ldev);                /* Select logical device */
    WbWrite(reg, val);                  /* Write logical device */
}

static inline xm_s32_t WbReadLdev(xm_s32_t ldev, xm_s32_t reg) {
    xm_s32_t val;

    WbWrite(0x07, ldev);                /* Select logical device */
    val = WbRead(reg);                  /* Read logical device */

    return val;
}

static xm_s32_t InitGpio(void) {
    extern xm_s32_t kprintf(const char *format,...);

    WbWrite(0x2A, 0xFC);                /* Enable GPIO */
    WbWriteLdev(0x07, 0x30, 0x01);      /* Activate GPIO */
    WbWriteLdev(0x07, 0xF0, 0x00);      /* Set GPIO pins as output */
    return 0;
}

static xm_s32_t WriteGpio(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len) {

    WbWriteLdev(0x07, 0xF1, *buffer);

    return len;
}

const kDevice_t gpioDev={
    .subId=0,
    .Reset=0,
    .Write=WriteGpio,
    .Read=0,
    .Seek=0,
};

static const kDevice_t *GetGpio(xm_u32_t subId) {
    switch(subId) {
    case 0:
        return &gpioDev;
        break;
    }

    return 0;
}

const struct kDevReg gpioReg={
    .id=XM_DEV_PC_WB_ID,
    .Init=InitGpio,
    .GetKDev=GetGpio,
};

REGISTER_KDEV_SETUP(gpioReg);

#endif
