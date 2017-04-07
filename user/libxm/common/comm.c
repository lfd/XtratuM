/*
 * $FILE: comm.c
 *
 * Communication wrappers
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <xm.h>
#include <comm.h>
#include <hypervisor.h>
#include <xmhypercalls.h>

#include <xm_inc/hypercalls.h>

__stdcall xm_s32_t XM_create_sampling_port(char *portName, xm_u32_t maxMsgSize, xm_u32_t direction) {
    xmObjDesc_t desc=OBJDESC_BUILD(OBJ_CLASS_SAMPLING_PORT, XM_PARTITION_SELF, 0);
    union samplingPortCmd cmd;
    xm_s32_t id;

    cmd.create.portName=portName;
    cmd.create.maxMsgSize=maxMsgSize;
    cmd.create.direction=direction;
    
    id=XM_ctrl_object(desc, XM_COMM_CREATE_PORT, &cmd);
    if (id<0)
        return XM_INVALID_CONFIG;
    desc=OBJDESC_SET_ID(desc, id);
    return desc;
}

__stdcall xm_s32_t XM_read_sampling_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize, xm_u32_t *flags) {
    if (OBJDESC_GET_CLASS(portId)!=OBJ_CLASS_SAMPLING_PORT)
	return XM_INVALID_PARAM;
    return XM_read_object(portId, msgPtr, msgSize, flags);
}

__stdcall xm_s32_t XM_write_sampling_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize) {
    if (OBJDESC_GET_CLASS(portId)!=OBJ_CLASS_SAMPLING_PORT)
	return XM_INVALID_PARAM;
    return XM_write_object(portId, msgPtr, msgSize, 0);
}

__stdcall xm_s32_t XM_create_queuing_port(char *portName, xm_u32_t maxNoMsgs, xm_u32_t maxMsgSize, xm_u32_t direction) {
    xmObjDesc_t desc=OBJDESC_BUILD(OBJ_CLASS_QUEUING_PORT, XM_PARTITION_SELF, 0);
    union queuingPortCmd cmd;
    xm_s32_t id;

    cmd.create.portName=portName;
    cmd.create.maxNoMsgs=maxNoMsgs;
    cmd.create.maxMsgSize=maxMsgSize;
    cmd.create.direction=direction;

    id=XM_ctrl_object(desc, XM_COMM_CREATE_PORT, &cmd);
    if (id<0)
        return XM_INVALID_CONFIG;
    desc=OBJDESC_SET_ID(desc, id);

    return desc;
}

__stdcall xm_s32_t XM_send_queuing_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize) {
    if (OBJDESC_GET_CLASS(portId)!=OBJ_CLASS_QUEUING_PORT)
      return XM_INVALID_PARAM;
    return XM_write_object(portId, msgPtr, msgSize, 0);
}

__stdcall xm_s32_t XM_receive_queuing_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize, xm_u32_t *flags) {
    if (OBJDESC_GET_CLASS(portId)!=OBJ_CLASS_QUEUING_PORT)
	return XM_INVALID_PARAM;
    return XM_read_object(portId, msgPtr, msgSize, flags);
}

__stdcall xm_s32_t XM_get_queuing_port_status(xm_u32_t portId, xmQueuingPortStatus_t *status) {
    if (OBJDESC_GET_CLASS(portId)!=OBJ_CLASS_QUEUING_PORT)
	return XM_INVALID_PARAM;
    if (!status)
	return XM_INVALID_PARAM;
    
    if (XM_ctrl_object(portId, XM_COMM_GET_PORT_STATUS, status)!=XM_OK)
	return XM_INVALID_PARAM;

    return XM_OK;
}

__stdcall xm_s32_t XM_get_sampling_port_status(xm_u32_t portId, xmSamplingPortStatus_t *status) {
    if (OBJDESC_GET_CLASS(portId)!=OBJ_CLASS_SAMPLING_PORT)
	return XM_INVALID_PARAM;
    if (!status)
	return XM_INVALID_PARAM;
    
    if (XM_ctrl_object(portId, XM_COMM_GET_PORT_STATUS, status)!=XM_OK)
	return XM_INVALID_PARAM;
    return XM_OK;
}

