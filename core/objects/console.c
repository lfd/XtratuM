/*
 * $FILE: console.c
 *
 * Object console
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
#include <sched.h>
#include <objdir.h>
#include <stdc.h>

#include <objects/console.h>

struct console {
    const kDevice_t *dev;
};

static struct console xmCon, *partitionConTab;

void __VBOOT ConsoleInit(const kDevice_t *kDev) { 
    if (kDev)
	xmCon.dev=kDev;
}


void ConsolePutChar(char c) {    
    if (KDevWrite(xmCon.dev, &c, 1)!=1) {
	KDevSeek(xmCon.dev, 0, DEV_SEEK_START);
	KDevWrite(xmCon.dev, &c, 1);
    }
}

static xm_s32_t ReadConsoleObj(xmObjDesc_t desc, char *buffer, xm_u32_t length) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmId_t partId=OBJDESC_GET_PARTITIONID(desc);

    if ((partId!=sched->cKThread->ctrl.g->cfg->id)&&!IS_KTHREAD_FLAG_SET(sched->cKThread, KTHREAD_SV_F))
	return XM_PERM_ERROR;
    
    return KDevRead(((partId==XM_HYPERVISOR_ID)?&xmCon:&partitionConTab[partId])->dev, buffer, length);
}

static xm_s32_t WriteConsoleObj(xmObjDesc_t desc, char *buffer, xm_u32_t length) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmId_t partId=OBJDESC_GET_PARTITIONID(desc);
    struct console *con;
    xm_s32_t e;

    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;

    con=(partId==XM_HYPERVISOR_ID)?&xmCon:&partitionConTab[partId];

    for (e=0; e<length; e++) {
	if (KDevWrite(con->dev, &buffer[e], 1)!=1) {
	    KDevSeek(con->dev, 0, DEV_SEEK_START);
	    if (KDevWrite(con->dev, &buffer[e], 1)!=1)
		return e;
	}
    }
    
    return length;    
}

static xm_s32_t SeekConsoleObj(xmObjDesc_t desc, xm_u32_t offset, xm_u32_t whence) {
    localSched_t *sched=GET_LOCAL_SCHED();
    xmId_t partId=OBJDESC_GET_PARTITIONID(desc);
    if (partId!=sched->cKThread->ctrl.g->cfg->id)
	return XM_PERM_ERROR;
    return KDevSeek(((partId==XM_HYPERVISOR_ID)?&xmCon:&partitionConTab[partId])->dev, offset, whence);
}

static const struct object consoleObj={
    .Write=(writeObjOp_t)WriteConsoleObj,
    .Read=(readObjOp_t)ReadConsoleObj,
    .Seek=(seekObjOp_t)SeekConsoleObj,    
};

xm_s32_t __VBOOT SetupConsole(void) {
    xm_s32_t e;

    GET_MEMZ(partitionConTab, sizeof(struct console)*xmcTab.noPartitions);
    xmCon.dev=LookUpKDev(&xmcTab.hpv.consoleDev);
    objectTab[OBJ_CLASS_CONSOLE]=&consoleObj;
    for (e=0; e<xmcTab.noPartitions; e++) {
	partitionConTab[e].dev=LookUpKDev(&xmcPartitionTab[e].consoleDev);
    }
    objectTab[OBJ_CLASS_CONSOLE]=&consoleObj;
    return 0;
}

REGISTER_OBJ(SetupConsole);
