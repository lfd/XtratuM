/*
 * $FILE: logstream.h
 *
 * Log Stream
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_LOGSTREAM_H_
#define _XM_LOGSTREAM_H_

#include <assert.h>
#include <kdevice.h>
#include <stdc.h>

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

struct logStream {
    xm_s32_t tail, elem, head, d, elemSize, maxNoElem;
    const kDevice_t *kDev;
};

static inline void LogStreamInit(struct logStream *lS, const kDevice_t *kDev, xm_s32_t elemSize) {
    ASSERT(elemSize>0);
    memset(lS, 0, sizeof(struct logStream));
    lS->elemSize=elemSize;
    lS->kDev=kDev;
    KDevReset(kDev);
    lS->maxNoElem=KDevSeek(lS->kDev, 0, DEV_SEEK_END);
    lS->maxNoElem=(lS->maxNoElem>0)?lS->maxNoElem/elemSize:0;
}

static inline xm_s32_t LogStreamInsert(struct logStream *lS, void *log) {
    xm_s32_t smashed=0;
    
    if (lS->maxNoElem) {
	if (lS->elem>=lS->maxNoElem) {
	    if (lS->elem) {
		lS->head=((lS->head+1)<lS->maxNoElem)?lS->head+1:0;
		lS->elem--;
	    }
	    smashed++;
	}
	
	if (lS->elem<lS->maxNoElem) {
	    KDevSeek(lS->kDev, lS->tail*lS->elemSize, DEV_SEEK_START);
	    KDevWrite(lS->kDev, log, lS->elemSize);
	    lS->tail=((lS->tail+1)<lS->maxNoElem)?lS->tail+1:0;
	    lS->elem++;
	}
    } else
	KDevWrite(lS->kDev, log, lS->elemSize);

    return smashed;
}

static inline xm_s32_t LogStreamExtract(struct logStream *lS, void *log) {
    if (lS->elem>0) {
	if (lS->elem) {
	    KDevSeek(lS->kDev, lS->head*lS->elemSize, DEV_SEEK_START);
	    KDevRead(lS->kDev, log, lS->elemSize);	  
	    lS->head=((lS->head+1)<lS->maxNoElem)?lS->head+1:0;
	    lS->elem--;
	}
	if (lS->d>0) lS->d--;
        return 0;
    }
    return -1;
}

static inline xm_s32_t LogStreamGet(struct logStream *lS, void *log) {
    xm_s32_t ptr;
    if ((lS->elem)>0&&(lS->d<lS->elem)) {
        ptr=(lS->d+lS->head)%lS->maxNoElem;
        lS->d++;
	KDevSeek(lS->kDev, ptr*lS->elemSize, DEV_SEEK_START);
	KDevRead(lS->kDev, log, lS->elemSize);
        return 0;
    }
    return -1;
}

// These values must match with the ones defined in hypercall.h
#define XM_LOGSTREAM_CURRENT 0x0
#define XM_LOGSTREAM_START 0x1
#define XM_LOGSTREAM_END 0x2

static inline xm_s32_t LogStreamSeek(struct logStream *lS,  xm_s32_t offset,  xm_u32_t whence) {
    xm_s32_t off=offset;
    switch((whence)) {
    case XM_LOGSTREAM_START:
	break;
    case XM_LOGSTREAM_CURRENT:
	off+=lS->d;
	break;
    case XM_LOGSTREAM_END:
	off+=lS->elem;
	break;
    default:
	return -1;
    }
    if (off>lS->elem) off=lS->elem;
    if (off<0) off=0;
    lS->d=off;
    return off;
}

#endif
