/*
 * $FILE: comm.c
 *
 * Communication wrappers
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include "xm-linux.h"

// Sampling port related functions
__stdcall xm_s32_t XM_create_sampling_port(char *portName, xm_u32_t maxMsgSize, xm_u32_t direction) {
    xmObjDesc_t desc = OBJDESC_BUILD(OBJ_CLASS_SAMPLING_PORT, XM_PARTITION_SELF, 0);
    union samplingPortCmd cmd = {
        .create = {
                   .portName = portName,
                   .maxMsgSize = maxMsgSize,
                   .direction = direction,
                   },
    };

    xm_s32_t id;
    id = XM_ctrl_object(desc, XM_COMM_CREATE_PORT, &cmd);
    if (id<0)
        return XM_INVALID_PARAM;
    desc = OBJDESC_SET_ID(desc, id);
    return desc;
}

__stdcall xm_s32_t XM_read_sampling_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize, xm_u32_t * flags) {
    if (OBJDESC_GET_CLASS(portId) != OBJ_CLASS_SAMPLING_PORT)
        return XM_INVALID_PARAM;
    return XM_read_object(portId, msgPtr, msgSize, flags);
}

__stdcall xm_s32_t XM_write_sampling_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize) {
    if (OBJDESC_GET_CLASS(portId) != OBJ_CLASS_SAMPLING_PORT)
        return XM_INVALID_PARAM;
    return XM_write_object(portId, msgPtr, msgSize, 0);
}

// Queuing port related functions
__stdcall xm_s32_t XM_create_queuing_port(char *portName, xm_u32_t maxNoMsgs, xm_u32_t maxMsgSize, xm_u32_t direction) {
    xmObjDesc_t desc = OBJDESC_BUILD(OBJ_CLASS_QUEUING_PORT, XM_PARTITION_SELF, 0);
    union queuingPortCmd cmd = {
        .create = {
                   .portName = portName,
                   .maxNoMsgs = maxNoMsgs,
                   .maxMsgSize = maxMsgSize,
                   .direction = direction,
                   },
    };
    xm_s32_t id;
    id = XM_ctrl_object(desc, XM_COMM_CREATE_PORT, &cmd);
    if (id<0)
        return XM_INVALID_PARAM;
    desc = OBJDESC_SET_ID(desc, id);
    return desc;
}

__stdcall xm_s32_t XM_send_queuing_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize) {
    if (OBJDESC_GET_CLASS(portId) != OBJ_CLASS_QUEUING_PORT)
        return XM_INVALID_PARAM;
    return XM_write_object(portId, msgPtr, msgSize, 0);
}

__stdcall xm_s32_t XM_receive_queuing_message(xm_s32_t portId, void *msgPtr, xm_u32_t msgSize, xm_u32_t * flags) {
    if (OBJDESC_GET_CLASS(portId) != OBJ_CLASS_QUEUING_PORT)
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
