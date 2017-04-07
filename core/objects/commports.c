/*
 * $FILE: commports.c
 *
 * Inter-partition communication mechanisms
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
#include <bitmap.h>
#include <boot.h>
#include <list.h>
#include <gaccess.h>
#include <kthread.h>
#include <hypercalls.h>
#include <brk.h>
#include <sched.h>
#include <stdc.h>
#include <xmconf.h>
#include <objects/commports.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif

static union channel {
    struct {
	char *buffer;
	xm_s32_t length;
	xmTime_t timeStamp;
    } s;
    struct {
	struct msg {
	    struct dynListNode listNode;
	    char *buffer;
	    xm_s32_t length;
	    xmTime_t timeStamp;
	} *msgPool;
	struct dynList freeMsgs, recvMsgs;
	xm_s32_t usedMsgs;
	kThread_t *receiver, *sender;
    } q;
} *channelTab;

static struct port {   
    xm_u32_t flags;
#define COMM_PORT_OPENED 0x1
    xmId_t partitionId;
} *portTab;

static inline xm_s32_t CreateSamplingPort(xmObjDesc_t desc, char *portName, xm_u32_t maxMsgSize, xm_u32_t direction) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcPartition *partition;
    xm_s32_t port;
    
    partition=sched->cKThread->ctrl.g->cfg;

    if (OBJDESC_GET_PARTITIONID(desc)!=partition->id) 
	return XM_PERM_ERROR;
    // Look for the channel
    for (port=partition->commPortsOffset; port<(partition->noPorts+partition->commPortsOffset); port++)
	if (!strcmp(portName, &xmcStringTab[xmcCommPorts[port].nameOffset])) break;
  
    if (port>=xmcTab.noCommPorts)
	return XM_INVALID_PARAM;

    if (xmcCommPorts[port].type!=XM_SAMPLING_PORT) 
	return XM_INVALID_PARAM;

    if (direction!=xmcCommPorts[port].direction)
	return XM_INVALID_PARAM;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	ASSERT((xmcCommPorts[port].channelId>=0)&&(xmcCommPorts[port].channelId<xmcTab.noCommChannels));
	if (xmcCommChannelTab[xmcCommPorts[port].channelId].s.maxLength!=maxMsgSize)
	    return XM_INVALID_CONFIG;
    }
    portTab[port].flags|=COMM_PORT_OPENED;
    portTab[port].partitionId=sched->cKThread->ctrl.g->cfg->id;
    return port;
}

static xm_s32_t ReadSamplingPort(xmObjDesc_t desc,  void *msgPtr, xmSize_t msgSize, xm_u32_t *flags) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t port=OBJDESC_GET_ID(desc);
    xmSize_t retSize=0;
    if (__CheckGParam(0, msgPtr, msgSize)<0) return XM_INVALID_PARAM;

    if (OBJDESC_GET_PARTITIONID(desc)!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;
    
    // Reading a port which does not belong to this partition
    if (portTab[port].partitionId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_INVALID_PARAM;
    
    if (!(portTab[port].flags&COMM_PORT_OPENED))
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].type!=XM_SAMPLING_PORT) 
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].direction!=XM_DESTINATION_PORT)
	return XM_INVALID_PARAM;
    
    if (!msgSize||!msgPtr)
	return XM_INVALID_CONFIG;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	if (msgSize>xmcCommChannelTab[xmcCommPorts[port].channelId].s.maxLength)
	    return XM_INVALID_CONFIG;
	retSize=(msgSize<channelTab[xmcCommPorts[port].channelId].s.length)?msgSize:channelTab[xmcCommPorts[port].channelId].s.length;
	memcpy(msgPtr, channelTab[xmcCommPorts[port].channelId].s.buffer, retSize);
#ifdef CONFIG_OBJ_STATUS_ACC
        systemStatus.noSamplingPortMsgsRead++;
        if (sched->cKThread->ctrl.g)
            partitionStatus[sched->cKThread->ctrl.g->cfg->id].noSamplingPortMsgsRead++;
#endif
	if (flags)
    	    if ((!xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod)||(channelTab[xmcCommPorts[port].channelId].s.timeStamp+xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod)<GetSysClockUsec())
    		*flags=XM_COMM_MSG_VALID;    	
    }
    return retSize;
}

static xm_s32_t WriteSamplingPort(xmObjDesc_t desc, void *msgPtr, xmSize_t msgSize) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t port=OBJDESC_GET_ID(desc);
    if (__CheckGParam(0, msgPtr, msgSize)<0) return XM_INVALID_PARAM;

    if (OBJDESC_GET_PARTITIONID(desc)!=sched->cKThread->ctrl.g->cfg->id) 
	return XM_PERM_ERROR;

    // Reading a port which does not belong to this partition    
    if (portTab[port].partitionId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_INVALID_PARAM;
    
    if (!(portTab[port].flags&COMM_PORT_OPENED))
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].type!=XM_SAMPLING_PORT) 
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].direction!=XM_SOURCE_PORT)
	return XM_INVALID_PARAM;

    if (!msgSize||!msgPtr)
	return XM_INVALID_CONFIG;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	if (msgSize>xmcCommChannelTab[xmcCommPorts[port].channelId].s.maxLength)
	    return XM_INVALID_CONFIG;
	
	memcpy(channelTab[xmcCommPorts[port].channelId].s.buffer, msgPtr, msgSize);
#ifdef CONFIG_OBJ_STATUS_ACC
        systemStatus.noSamplingPortMsgsWritten++;
        if (sched->cKThread->ctrl.g)
            partitionStatus[sched->cKThread->ctrl.g->cfg->id].noSamplingPortMsgsWritten++;
#endif
	channelTab[xmcCommPorts[port].channelId].s.length=msgSize;
	if (xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod)
	    channelTab[xmcCommPorts[port].channelId].s.timeStamp=GetSysClockUsec();	
    }
   return XM_OK;
}

static inline xm_s32_t GetSPortStatus(xmObjDesc_t desc, xmSamplingPortStatus_t *status) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t port=OBJDESC_GET_ID(desc);
    if (OBJDESC_GET_PARTITIONID(desc)!=sched->cKThread->ctrl.g->cfg->id) 
	return XM_PERM_ERROR;

    // Reading a port which does not belong to this partition    
    if (portTab[port].partitionId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_INVALID_PARAM;
    
    if (!(portTab[port].flags&COMM_PORT_OPENED))
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].type!=XM_SAMPLING_PORT) 
	return XM_INVALID_PARAM;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	status->validPeriod=xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod;
	status->maxMsgSize=xmcCommChannelTab[xmcCommPorts[port].channelId].s.maxLength;
	status->flags=0;
    } else
	memset(status, 0, sizeof(xmSamplingPortStatus_t));
    
    return XM_OK;
}

static xm_s32_t CtrlSamplingPort(xmObjDesc_t desc, xm_u32_t cmd, union samplingPortCmd *__gParam args) {
    if (!args)
	return XM_INVALID_PARAM;
    
    if (__CheckGParam(0, args, sizeof(union samplingPortCmd))<0) return XM_INVALID_PARAM;
    switch(cmd) {
    case XM_COMM_CREATE_PORT:
	if (!args->create.portName||(__CheckGParam(0, args->create.portName, strlen(args->create.portName))<0)) return XM_INVALID_PARAM;
	return CreateSamplingPort(desc, args->create.portName, args->create.maxMsgSize, args->create.direction);
	break;
    case XM_COMM_GET_PORT_STATUS:
	return GetSPortStatus(desc, &args->status);
	break;
    }
    return XM_INVALID_PARAM;
}

static const struct object samplingPortObj={
    .Read=(readObjOp_t)ReadSamplingPort,
    .Write=(writeObjOp_t)WriteSamplingPort,
    .Ctrl=(ctrlObjOp_t)CtrlSamplingPort,
};

static inline xm_s32_t CreateQueuingPort(xmObjDesc_t desc, char *portName, xm_s32_t maxNoMsgs, xm_s32_t maxMsgSize, xm_u32_t direction) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcPartition *partition;
    xm_s32_t port;

    partition=sched->cKThread->ctrl.g->cfg;
    if (OBJDESC_GET_PARTITIONID(desc)!=partition->id) 
	return XM_PERM_ERROR;

    // Look for the channel
    for (port=partition->commPortsOffset; port<(partition->noPorts+partition->commPortsOffset); port++)
	if (!strcmp(portName, &xmcStringTab[xmcCommPorts[port].nameOffset])) break;
  
    if (port>=xmcTab.noCommPorts)
	return XM_INVALID_PARAM;

    if (xmcCommPorts[port].type!=XM_QUEUING_PORT) 
	return XM_INVALID_PARAM;

    if (direction!=xmcCommPorts[port].direction)
	return XM_INVALID_PARAM;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	ASSERT((xmcCommPorts[port].channelId>=0)&&(xmcCommPorts[port].channelId<xmcTab.noCommChannels));

	if (direction==XM_DESTINATION_PORT)
	    channelTab[xmcCommPorts[port].channelId].q.receiver=sched->cKThread;
	if (direction==XM_SOURCE_PORT)
	    channelTab[xmcCommPorts[port].channelId].q.sender=sched->cKThread;

	if (xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxNoMsgs!=maxNoMsgs)
	    return XM_INVALID_CONFIG;
	if (xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxLength!=maxMsgSize)
	    return XM_INVALID_CONFIG;	
    }
    portTab[port].flags|=COMM_PORT_OPENED;
    portTab[port].partitionId=sched->cKThread->ctrl.g->cfg->id;

    return port;
}

static xm_s32_t SendQueuingPort(xmObjDesc_t desc, void *msgPtr, xm_u32_t msgSize) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t port=OBJDESC_GET_ID(desc);
    struct msg *msg;

    if (__CheckGParam(0, msgPtr, msgSize)<0) return XM_INVALID_PARAM;
    if (OBJDESC_GET_PARTITIONID(desc)!=sched->cKThread->ctrl.g->cfg->id) 
	return XM_PERM_ERROR;

    // Reading a port which does not belong to this partition    
    if (portTab[port].partitionId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_INVALID_PARAM;
    if (!(portTab[port].flags&COMM_PORT_OPENED))
	return XM_INVALID_PARAM;
    if (xmcCommPorts[port].type!=XM_QUEUING_PORT) 
	return XM_INVALID_PARAM;
    if (xmcCommPorts[port].direction!=XM_SOURCE_PORT)
	return XM_INVALID_PARAM;
    if (!msgSize||!msgPtr)
	return XM_INVALID_CONFIG;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	if (msgSize>xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxLength)
	    return XM_INVALID_CONFIG;

	if (channelTab[xmcCommPorts[port].channelId].q.usedMsgs<xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxNoMsgs) {
    	    if (!(msg=(struct msg *)DynListRemoveTail(&channelTab[xmcCommPorts[port].channelId].q.freeMsgs)))
    		SystemPanic(0, 0, "[SendQueuingPort] Queuing channels internal error");
    	    memcpy(msg->buffer, msgPtr, msgSize);
#ifdef CONFIG_OBJ_STATUS_ACC
            systemStatus.noQueuingPortMsgsSent++;
            if (sched->cKThread->ctrl.g)
                partitionStatus[sched->cKThread->ctrl.g->cfg->id].noQueuingPortMsgsSent++;
#endif
	    msg->length=msgSize;
    	    if (xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod)
    		msg->timeStamp=GetSysClockUsec();
    	    DynListInsertHead(&channelTab[xmcCommPorts[port].channelId].q.recvMsgs, &msg->listNode);
    	    channelTab[xmcCommPorts[port].channelId].q.usedMsgs++;

	    if (channelTab[xmcCommPorts[port].channelId].q.receiver) {
		XMAtomicSetMask(1<<OBJ_CLASS_QUEUING_PORT, &channelTab[xmcCommPorts[port].channelId].q.receiver->ctrl.g->partitionControlTable->objDescClassPend);
		SetExtIrqPending(channelTab[xmcCommPorts[port].channelId].q.receiver, XM_VT_EXT_OBJDESC);
	    }
    	} else
    	    return XM_NOT_AVAILABLE;
    }

    return XM_OK;
}

static xm_s32_t ReceiveQueuingPort(xmObjDesc_t desc, void *msgPtr, xm_u32_t msgSize, xm_u32_t *flags) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t port=OBJDESC_GET_ID(desc);
    xmSize_t retSize=0;
    struct msg *msg;

    if (__CheckGParam(0, msgPtr, msgSize)<0) return XM_INVALID_PARAM;
    
    if (OBJDESC_GET_PARTITIONID(desc)!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;
    
    // Reading a port which does not belong to this partition
    if (portTab[port].partitionId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_INVALID_PARAM;
    
    if (!(portTab[port].flags&COMM_PORT_OPENED))
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].type!=XM_QUEUING_PORT) 
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].direction!=XM_DESTINATION_PORT)
	return XM_INVALID_PARAM;
    
    if (!msgSize||!msgPtr)
	return XM_INVALID_CONFIG;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	if (msgSize>xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxLength)
	    return XM_INVALID_CONFIG;

	if (channelTab[xmcCommPorts[port].channelId].q.usedMsgs>0) {
	    if (!(msg=(struct msg *)DynListRemoveTail(&channelTab[xmcCommPorts[port].channelId].q.recvMsgs)))
    		SystemPanic(0, 0, "[ReceiveQueuingPort] Queuing channels internal error");
	    retSize=(msgSize<msg->length)?msgSize:msg->length;
	    memcpy(msgPtr, msg->buffer, retSize);
#ifdef CONFIG_OBJ_STATUS_ACC
            systemStatus.noQueuingPortMsgsReceived++;
            if (sched->cKThread->ctrl.g)
                partitionStatus[sched->cKThread->ctrl.g->cfg->id].noQueuingPortMsgsReceived++;
#endif
	    DynListInsertHead(&channelTab[xmcCommPorts[port].channelId].q.freeMsgs, &msg->listNode);
    	    channelTab[xmcCommPorts[port].channelId].q.usedMsgs--;
	    if (flags)
		if ((!xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod)||(msg->timeStamp+xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod)<GetSysClockUsec())
    		*flags=XM_COMM_MSG_VALID;
	    if (channelTab[xmcCommPorts[port].channelId].q.sender) {
	            XMAtomicSetMask(1<<OBJ_CLASS_QUEUING_PORT, &channelTab[xmcCommPorts[port].channelId].q.sender->ctrl.g->partitionControlTable->objDescClassPend);
	            SetExtIrqPending(channelTab[xmcCommPorts[port].channelId].q.sender, XM_VT_EXT_OBJDESC);
	            }
	} else
	    return XM_NOT_AVAILABLE;
    }

    return retSize;      
}

static inline xm_s32_t GetQPortStatus(xmObjDesc_t desc, xmQueuingPortStatus_t *status) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xm_s32_t port=OBJDESC_GET_ID(desc);
    if (OBJDESC_GET_PARTITIONID(desc)!=sched->cKThread->ctrl.g->cfg->id) 
	return XM_PERM_ERROR;

    // Reading a port which does not belong to this partition    
    if (portTab[port].partitionId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_INVALID_PARAM;
    
    if (!(portTab[port].flags&COMM_PORT_OPENED))
	return XM_INVALID_PARAM;
    
    if (xmcCommPorts[port].type!=XM_QUEUING_PORT)
	return XM_INVALID_PARAM;

    if (xmcCommPorts[port].channelId!=XM_NULL_CHANNEL) {
	status->validPeriod=xmcCommChannelTab[xmcCommPorts[port].channelId].validPeriod;
	status->maxMsgSize=xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxLength;
	status->maxNoMsgs=xmcCommChannelTab[xmcCommPorts[port].channelId].q.maxNoMsgs;
	status->flags=0;
    } else
	memset(status, 0, sizeof(xmSamplingPortStatus_t));
    
    return XM_OK;
}

static xm_s32_t CtrlQueuingPort(xmObjDesc_t desc, xm_u32_t cmd, union queuingPortCmd *__gParam args) {
    if (!args)
	return XM_INVALID_PARAM;
    
    if (__CheckGParam(0, args, sizeof(union queuingPortCmd))<0) return XM_INVALID_PARAM;

    switch(cmd) {
    case XM_COMM_CREATE_PORT:
	if (!args->create.portName||(__CheckGParam(0, args->create.portName, strlen(args->create.portName))<0)) return XM_INVALID_PARAM;
	return CreateQueuingPort(desc, args->create.portName, args->create.maxNoMsgs, args->create.maxMsgSize, args->create.direction);
	break;
    case XM_COMM_GET_PORT_STATUS:
	return GetQPortStatus(desc, &args->status);
	break;
    }
    return XM_INVALID_PARAM;
}

static const struct object queuingPortObj={
    .Read=(readObjOp_t)ReceiveQueuingPort,
    .Write=(writeObjOp_t)SendQueuingPort,    
    .Ctrl=(ctrlObjOp_t)CtrlQueuingPort,
};

xm_s32_t __VBOOT SetupComm(void) {
    xm_s32_t e, i;

    GET_MEMZ(channelTab, sizeof(union channel)*xmcTab.noCommChannels);
    GET_MEMZ(portTab, sizeof(struct port)*xmcTab.noCommPorts);

    /* create the channels */
    for (e=0; e<xmcTab.noCommChannels; e++) {
	switch(xmcCommChannelTab[e].type) {
	case XM_SAMPLING_CHANNEL:
	    GET_MEMZ(channelTab[e].s.buffer, xmcCommChannelTab[e].s.maxLength);
	    break;
	case XM_QUEUING_CHANNEL:
	    GET_MEMZ(channelTab[e].q.msgPool, sizeof(struct msg)*xmcCommChannelTab[e].q.maxNoMsgs);
	    DynListInit(&channelTab[e].q.freeMsgs);
	    DynListInit(&channelTab[e].q.recvMsgs);
	    for (i=0; i<xmcCommChannelTab[e].q.maxNoMsgs; i++) {
		GET_MEMZ(channelTab[e].q.msgPool[i].buffer, xmcCommChannelTab[e].q.maxLength);
		if(DynListInsertHead(&channelTab[e].q.freeMsgs, &channelTab[e].q.msgPool[i].listNode))
		    SystemPanic(0, 0, "[SetupComm] Queuing channels initialisation error");
	    }
	    break;
	}
    }

    objectTab[OBJ_CLASS_SAMPLING_PORT]=&samplingPortObj;
    objectTab[OBJ_CLASS_QUEUING_PORT]=&queuingPortObj;
    
    return 0;
}
  
REGISTER_OBJ(SetupComm);
