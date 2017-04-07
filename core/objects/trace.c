/*
 * $FILE: trace.c
 *
 * Tracing mechanism
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <brk.h>
#include <boot.h>
#include <hypercalls.h>
#include <kthread.h>
#include <kdevice.h>
#include <brk.h>
#include <stdc.h>
#include <xmconf.h>
#include <sched.h>
#include <objects/trace.h>
#include <objects/hm.h>
#include <logstream.h>

static struct logStream *traceLogStream, xmTraceLogStream;

static xm_s32_t ReadTrace(xmObjDesc_t desc, xmTraceEvent_t *event, xm_u32_t size) {
    xm_s32_t e, noTraces=size/sizeof(xmTraceEvent_t);
    localSched_t *sched=GET_LOCAL_SCHED();
    struct logStream *log;
    xmId_t partId;
    
    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;

    if (!event || !noTraces)
        return XM_INVALID_PARAM;
    if (__CheckGParam(0, event, sizeof(*event)*noTraces)<0)
        return XM_INVALID_PARAM;

    if (partId==XM_HYPERVISOR_ID)
	log=&xmTraceLogStream;
    else {
	if ((partId<0)||(partId>=xmcTab.noPartitions))
	    return XM_INVALID_PARAM;
	log=&traceLogStream[partId];
    }
    
    for (e=0; e<noTraces; e++)
	if (LogStreamGet(log, &event[e])<0)
	    return e*sizeof(xmTraceEvent_t);
    
    return noTraces*sizeof(xmTraceEvent_t);
}

static xm_s32_t WriteTrace(xmObjDesc_t desc, xmTraceEvent_t *event, xm_u32_t size, xm_u32_t *bitmap) {
    xm_s32_t e, noTraces=size/sizeof(xmTraceEvent_t), written;
    localSched_t *sched=GET_LOCAL_SCHED();
    struct logStream *log;
    xmId_t partId;
    
    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;
    
    if (!event||!noTraces)
	return XM_INVALID_PARAM;
    
    log=&traceLogStream[partId];
    for (written=0, e=0; e<noTraces; e++) {
    	if (bitmap&&(sched->cKThread->ctrl.g->cfg->trace.bitmap&*bitmap)) {
	    event[e].opCode.partitionId=sched->cKThread->ctrl.g->cfg->id;
	    event[e].timeStamp=GetSysClockUsec();
	    LogStreamInsert(log, &event[e]);  
	    written++;
	}
	if (event->opCode.criticality==XM_TRACE_UNRECOVERABLE) {
	    xmHmLog_t log;
	    log.partitionId=sched->cKThread->ctrl.g->cfg->id;
	    log.system=0;
	    log.eventId=XM_HM_EV_PARTITION_ERROR;
	    log.moduleId=event->opCode.moduleId;
	    log.word[0]=event->opCode.code;
	    log.word[1]=event->word[0];
	    log.word[2]=event->word[1];
	    log.word[3]=event->word[2];
	    log.word[4]=event->word[3];

	    HmRaiseEvent(&log);
	}
    }

    return written*sizeof(xmTraceEvent_t);
}

static xm_s32_t SeekTrace(xmObjDesc_t desc, xm_u32_t offset, xm_u32_t whence) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct logStream *log;
    xmId_t partId;
    
    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	    return XM_PERM_ERROR;

    if (partId==XM_HYPERVISOR_ID)
	log=&xmTraceLogStream;
    else {
	if ((partId<0)||(partId>=xmcTab.noPartitions))
	    return XM_INVALID_PARAM;
	log=&traceLogStream[partId];
    }

    return LogStreamSeek(log, offset, whence);
}

static xm_s32_t CtrlTrace(xmObjDesc_t desc, xm_u32_t cmd, union traceCmd *__gParam args) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct logStream *log;
    xmId_t partId;
    
    if (OBJDESC_GET_ID(desc))
        return XM_INVALID_PARAM;
    partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
        if (!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
            return XM_PERM_ERROR;
    if (!args || __CheckGParam(0, args, sizeof(union traceCmd))<0)
        return XM_INVALID_PARAM;

    if (partId==XM_HYPERVISOR_ID)
        log=&xmTraceLogStream;
    else {
        if ((partId<0)||(partId>=xmcTab.noPartitions))
            return XM_INVALID_PARAM;
        log=&traceLogStream[partId];
    }

    switch(cmd) {
    default:
	    break;
    case XM_TRACE_GET_STATUS:
        args->status.noEvents=log->elem;
        args->status.maxEvents=log->maxNoElem;
        args->status.currentEvent=log->d;
	    return XM_OK;
    }
    return XM_INVALID_PARAM;
}

static const struct object traceObj={
    .Read=(readObjOp_t)ReadTrace,
    .Write=(writeObjOp_t)WriteTrace,
    .Seek=(seekObjOp_t)SeekTrace,
    .Ctrl=(ctrlObjOp_t)CtrlTrace,
};

xm_s32_t __VBOOT SetupTrace(void) {
    xm_s32_t e;

    GET_MEMZ(traceLogStream, sizeof(struct logStream)*xmcTab.noPartitions);
    LogStreamInit(&xmTraceLogStream, LookUpKDev(&xmcTab.hpv.trace.dev), sizeof(xmTraceEvent_t));
    
    for (e=0; e<xmcTab.noPartitions; e++)
	LogStreamInit(&traceLogStream[e], LookUpKDev(&xmcPartitionTab[e].trace.dev), sizeof(xmTraceEvent_t));
    
    objectTab[OBJ_CLASS_TRACE]=&traceObj;
    return 0;
}

xm_s32_t TraceWriteSysEvent(xm_u32_t bitmap, xmTraceEvent_t *event) {
    ASSERT(event);
    if (xmcTab.hpv.trace.bitmap&bitmap) {
	event->opCode.partitionId=XM_HYPERVISOR_ID;
	event->timeStamp=GetSysClockUsec();
	if (KDevWrite(xmTraceLogStream.kDev, (xm_u8_t *)event, sizeof(xmTraceEvent_t))==sizeof(xmTraceEvent_t)) return 1;
    }

    return 0;
}

REGISTER_OBJ(SetupTrace);
