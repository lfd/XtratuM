/*
 * $FILE: xmmanager-comm.c
 *
 * XM Partition Manager
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include "common.h"

static int CmdPortQueuingReader(char *portname, int portsize, int portnomsg) {
    int i, port, ret;
    char message[512];

    ret=0;
    if (portsize > sizeof(message)) {
        xmprintf("Error: max portsize is %d\n", sizeof(message));
        return -1;
    }

    if ((port = XM_create_queuing_port(portname, portsize, XM_DESTINATION_PORT, 0)) < 0) {
        xmprintf("Error: XM_create_queuing_port %s %d: %d\n", portname, portsize, port);
        return port;
    }

    for(i=0; i < portnomsg; i++) {
#ifdef sparcv8
        ret = XM_receive_queuing_message(port, message, portsize);
#else
        ret = XM_receive_queuing_message(port, message, portsize, 0);
#endif
        xmprintf("queuing: receive %d %s\n", ret, message);
    }
    return ret;
}

static int CmdPortQueuingWriter(char *portname, int portsize, char *message) {
    int port, ret;

    if ((port = XM_create_queuing_port(portname, portsize, XM_SOURCE_PORT, 0)) < 0) {
        xmprintf("Error: XM_create_queuing_port %s %d: %d\n", portname, portsize, port);
        return port;
    }

    ret = XM_send_queuing_message(port, message, sizeof(message));
    xmprintf("queuing_writer: wrote %d %s\n", ret, message);
    return ret;
}

static int CmdPortSamplingReader(char *portname, int portsize, int portnomsg) {
    int i, port, ret;
    char message[512];

    ret=0;
    if (portsize > sizeof(message)) {
        xmprintf("Error: max portsize is %d\n", sizeof(message));
        return -1;
    }

    if ((port = XM_create_sampling_port(portname, portsize, XM_DESTINATION_PORT)) < 0) {
        xmprintf("Error: XM_create_sampling_port %s %d: %d\n", portname, portsize, port);
        return port;
    }

    for(i=0; i < portnomsg; i++) {
        ret = XM_read_sampling_message(port, message, portsize, 0);
        xmprintf("sampling: read %d %s\n", ret, message);
    }
    return ret;
}

static int CmdPortSamplingWriter(char *portname, int portsize, char *message) {
    int port, ret;

    if ((port = XM_create_sampling_port(portname, portsize, XM_SOURCE_PORT)) < 0) {
        xmprintf("Error: XM_create_sampling_port %s %d: %d\n", portname, portsize, port);
        return port;
    }
    
    ret = XM_write_sampling_message(port, message, sizeof(message));
    xmprintf("sampling_writer: wrote %d %s\n", ret, message);
    return ret;
}

int CmdPort(char *line) {
    int narg, ret;
    char *arg[5];
    char *portname;
    int portsize;
    char *portmsg;

    int porttype, portoperation;
    char *porttypes[] = {"sampling", "queueing" };
    char *portoperations[] = {"read", "write"};

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: porttype not set\n");
        return -1;
    }else if(narg == 0){
        xmprintf("Error: portoperation not set\n");
        return -1;
    }else if(narg == 1){
        xmprintf("Error: portname not set\n");
        return -1;
    }else if(narg == 2){
        xmprintf("Error: portsize not set\n");
        return -1;
    }else if(narg == 3){
        xmprintf("Error: message not set\n");
        return -1;
    }

    if(narg != NELEM(arg)-1)
        return -1;

    porttype=GetCommandIndex(arg[0], porttypes, NELEM(porttypes));
    portoperation=GetCommandIndex(arg[1], portoperations, NELEM(portoperations));
    portname = arg[2];
    portsize = atoi(arg[3]);
    portmsg = arg[4];

    switch (portoperation) {
    default:
        xmprintf("Error: unknown portoperation\n");
        ret=-1;
        break;

    case 0:
        
        if(porttype == 0)
            ret=CmdPortSamplingReader(portname, portsize, atoi(portmsg));
        else if(porttype == 1)
            ret=CmdPortQueuingReader(portname, portsize, atoi(portmsg));
        else
            ret=-1;
        break;

    case 1:
        if(porttype == 0)
            ret=CmdPortSamplingWriter(portname, portsize, portmsg);
        else if(porttype == 1)
            ret=CmdPortQueuingWriter(portname, portsize, portmsg);
        else
            ret=-1;
        break;
    }

    return ret;
}
