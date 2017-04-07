/*
 * $FILE: kdevice.h
 *
 * kernel devices
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_KDEVICE_H_
#define _XM_KDEVICE_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <xmconf.h>
#include <devid.h>
#include <linkage.h>

typedef struct kDev {
    xm_u16_t subId;
    xm_s32_t (*Reset)(const struct kDev *);
    xm_s32_t (*Write)(const struct kDev *, xm_u8_t *buffer, xm_s32_t len);
    xm_s32_t (*Read)(const struct kDev *, xm_u8_t *buffer, xm_s32_t len);
    xm_s32_t (*Seek)(const struct kDev *, xm_u32_t offset, xm_u32_t whence);
} kDevice_t;

#define DEV_SEEK_CURRENT 0x0
#define DEV_SEEK_START 0x1
#define DEV_SEEK_END 0x2

#define KDEV_OK 0
#define KDEV_OP_NOT_ALLOWED 1

static inline xm_s32_t KDevReset(const kDevice_t *kDev) {
    if(kDev&&(kDev->Reset))
	return kDev->Reset(kDev);
    return -KDEV_OP_NOT_ALLOWED;
}

static inline xm_s32_t KDevWrite(const kDevice_t *kDev, void *buffer, xm_s32_t len) {
    if(kDev&&(kDev->Write))
	return kDev->Write(kDev, buffer, len);
    return -KDEV_OP_NOT_ALLOWED;
}

static inline xm_s32_t KDevRead(const kDevice_t *kDev, void *buffer, xm_s32_t len) {
    if(kDev&&(kDev->Read))
	return kDev->Read(kDev, buffer, len);
    return -KDEV_OP_NOT_ALLOWED;
}

static inline xm_s32_t KDevSeek(const kDevice_t *kDev, xm_u32_t offset, xm_u32_t whence) {
    if(kDev&&(kDev->Seek))
	return kDev->Seek(kDev, offset, whence);
    return -KDEV_OP_NOT_ALLOWED;
}

struct kDevReg {
    xm_u32_t id;
    xm_s32_t (*Init)(void);
    void (*Shutdown)(void);
    const kDevice_t *(*GetKDev)(xm_u32_t subId);
};

extern const kDevice_t *LookUpKDev(const xmDev_t *dev);
extern void SetupKDev(void);

#ifdef CONFIG_EARLY_KOUTPUT
extern const kDevice_t *GetEarlyOutput(void);
#else
#define GetEarlyOutput() ((void*)0)
#endif

#define REGISTER_KDEV_SETUP(_devReg) \
    __asm__ (".section .kdevtab, \"a\"\n\t" \
             ".align 4\n\t" \
             ".long "#_devReg"\n\t" \
             ".previous\n\t")

#define RESERVE_HWIRQ(_irq) \
    __asm__ (".section .rsv_hwirqs, \"a\"\n\t" \
             ".align 4\n\t" \
             ".long "TO_STR(_irq)"\n\t"	\
             ".previous\n\t")

#define RESERVE_IOPORTS(_base, _offset) \
    __asm__ (".section .rsv_ioports, \"a\"\n\t" \
             ".align 4\n\t" \
             ".long "TO_STR(_base)"\n\t"  \
	     ".long "TO_STR(_offset)"\n\t" \
             ".previous\n\t")

#define RESERVE_PHYSPAGES(_addr, _nPag) \
    __asm__ (".section .rsv_physpages, \"a\"\n\t" \
             ".align 4\n\t" \
             ".long "TO_STR(_addr)"\n\t"  \
	     ".long "TO_STR(_nPag)"\n\t" \
             ".previous\n\t")

#endif
