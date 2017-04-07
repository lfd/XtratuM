/*
 * $FILE: xmmanager.c
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
#include "xmmanager.h"

int CmdList(char *line) {
    int i, ret;
    xm_u32_t magic, srcaddr;
    xmPartitionStatus_t status;

    xmprintf("List of Partitions:\n");
    for (i = 0; 1; i++) {
#ifdef sparcv8
        ret = XM_get_partition_status(i, &status);
#else
        ret = XM_partition_get_status(i, &status);
#endif
        if (ret < 0)
            break;

        // TODO add partition bare/linux detection
        magic=srcaddr=0;
        //srcaddr=0x20000 + i * 0x20000;
        //if(i == XM_PARTITION_SELF)
        //    memcpy(&magic, (xm_u32_t*)srcaddr, sizeof(magic));
        //else
        //    ret=XM_memory_copy(XM_PARTITION_SELF, (xm_u32_t)&magic, i, srcaddr, sizeof(magic));

        xmprintf("%s Partition%d: 0x%x magic 0x%x state 0x%x \"%s\"\n",
            (i == XM_PARTITION_SELF)? "*":" ",
            i, srcaddr,  magic, status.state, StateToStr(status.state));
    }
    if (i > 0)
        nopart = i - 1;

    return 0;
}

int CmdPartition(char *line) {
    int narg, ret, partid=0;
    char *arg[4];
    char *operlist[] = {"halt", "reset", "resume", "status", "suspend"};
    char *resetlist[] = {"cold", "warm"};
    xmPartitionStatus_t status;
    xm_u32_t resetStatus = 0;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg <= 0)
        return 0;

    partid = atoi(arg[1]);
    if (CheckPartition(partid, CmdPartition) < 0)
        return 0;

    switch(GetCommandIndex(arg[0], operlist, NELEM(operlist))){
    case 0:
        ret=XM_halt_partition(partid);
        break;
    case 1:
        if(narg == 3)
            resetStatus = atoi(arg[3]);
        switch(GetCommandIndex(arg[2], resetlist, NELEM(resetlist))){
        case 0:
            ret=XM_reset_partition(partid, XM_COLD_RESET, resetStatus);
            break;
        case 1:
            ret=XM_reset_partition(partid, XM_WARM_RESET, resetStatus);
            break;
        default:
            ret=XM_reset_partition(partid, XM_WARM_RESET, resetStatus);
            break;
        }
        break;
    case 2:
        ret=XM_resume_partition(partid);
        break;
    case 3:
#ifdef sparcv8
        ret=XM_get_partition_status(partid, &status);
#else
        ret=XM_partition_get_status(partid, &status);
#endif
        xmprintf("execClock: %lld state: %d virqs: %lld\n"
            "resetCounter: %d resetStatus: %d\n"
            "noQueuingPortMsgsReceived %lld noSamplingPortMsgRead: %lld\n"
            "noSamplingPortMsgsWritten %lld noQueuingPortMsgsSent %lld\n",
            status.execClock, status.state, status.noVIrqs,
            status.resetCounter, status.resetStatus,
            status.noSamplingPortMsgsRead, status.noSamplingPortMsgsWritten,
            status.noQueuingPortMsgsSent, status.noQueuingPortMsgsReceived);
        break;
    case 4:
        ret=XM_suspend_partition(partid);
        break;

    default:
        return -1;
    }
    
    return ret;
}

static int XDump(void *va, void *pa, int size)
{
	int i;

	for (i = 0; i < size/4; i++){
		unsigned int * vptr = va + i*4;
		unsigned int * pptr = pa + i*4;
		xmprintf("%#08x %#08x\n", (unsigned int)pptr, *vptr);
	}
	return 0;
}

#ifdef linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGESHIFT 12
#define PAGESIZE (1<<PAGESHIFT)
#define PFN(addr) ((unsigned int)addr / PAGESIZE)
#define PAGEOFF(addr) ((unsigned int)addr & (PAGESIZE-1))
static unsigned int Va2Pa(void *va)
{
	int fd;
	char f[256];
	unsigned int pe[2]; /* page entry: 64-bits */

	snprintf(f, sizeof(f), "/proc/%d/pagemap", getpid());
	if ((fd = open(f, O_RDONLY)) < 0)
		return 0;
	if (lseek(fd, (off_t) PFN(va) * sizeof(pe), SEEK_SET) < 0)
		return 0;
	if (read(fd, pe, sizeof(pe)) != sizeof(pe))
		return 0;
	if (pe[1] != 0x86000000) /* page flags: present and not swapped */
		return 0;
	return pe[0] << PAGESHIFT | PAGEOFF(va);
}
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
static char buf[512];
int CmdXDump(char *line) {
    int narg;
    char *arg[3];
    int size, n, ret;
    int srcpart;
    int dstpart;
    xm_u32_t srcaddr;
    xm_u32_t dstaddr;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: srcpart not set\n");
        return -1;
    }else if(narg == 0){
        xmprintf("Error: srcaddr not set\n");
        return -1;
    }else if(narg == 1){
        xmprintf("Error: size not set\n");
        return -1;
    }

    srcpart = atoi(arg[0]);
    sscanf(arg[1], "%x", &srcaddr);
    size = atoi(arg[2]);
    
	dstpart = XM_PARTITION_SELF;
    dstaddr = Va2Pa(buf);
	if(!dstaddr || size < 0)
        return -1;
    
    for(; size > 0; size -= n, srcaddr += n){
        memset(buf, 0, sizeof(buf));
        n = MIN(size, sizeof(buf));
        if(srcpart == dstpart)
            ret=XM_memory_copy(dstpart, srcaddr, srcpart, dstaddr, n);
        else
            ret=XM_memory_copy(dstpart, dstaddr, srcpart, srcaddr, n);

        if(ret<0)
            xmprintf("XM_memory_copy(%d, %x, %x, %x, %d): ret %d\n",
                    dstpart, dstaddr, srcpart, srcaddr, n, ret);
        XDump(buf, (void*)srcaddr, n);
    }

    return ret;
}

#ifdef linux
int CmdXWrite(char *line) {
    int narg, ret;
    char *arg[3];
    int srcpart;
    int dstpart;
    xm_u32_t srcaddr;
    xm_u32_t dstaddr;
    xm_u32_t *value = (unsigned int*)buf;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: srcpart not set\n");
        return -1;
    }else if(narg == 0){
        xmprintf("Error: srcaddr not set\n");
        return -1;
    }else if(narg == 1){
        xmprintf("Error: value not set\n");
        return -1;
    }

    dstpart = atoi(arg[0]);
    sscanf(arg[1], "%x", &dstaddr);
    sscanf(arg[2], "%x", value);

	srcpart = XM_PARTITION_SELF;
	srcaddr = Va2Pa(value);

	if(!dstaddr || !srcaddr)
		return -1;

	ret=XM_memory_copy(dstpart, srcaddr, srcpart, dstaddr, sizeof(*value)); // inverted src/dst addrs!
	if(ret<0)
		xmprintf("XM_memory_copy(%d, %x, %x, %x, %d): ret %d\n",
			dstpart, dstaddr, srcpart, srcaddr, sizeof(*value), ret);
	XDump(value, (void*)dstaddr, sizeof(*value));
	return 0;
}

int CmdMemWrite(char *line)
{
    int narg, ret;
    char *arg[3];
    int srcpart;
    int dstpart;
    xm_u32_t srcaddr;
    xm_u32_t dstaddr;
    unsigned int *value = (unsigned int*)buf;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: dstpart not set\n");
        return -1;
    }else if(narg == 0){
        xmprintf("Error: dstaddr not set\n");
        return -1;
    }else if(narg == 1){
        xmprintf("Error: value not set\n");
        return -1;
    }

    dstpart = atoi(arg[0]);
    sscanf(arg[1], "%x", &dstaddr);
    *value = atoi(arg[2]);

	srcpart = XM_PARTITION_SELF;
	srcaddr = Va2Pa(value);

	if(!dstaddr || !srcaddr)
		return -1;

	ret=XM_memory_copy(dstpart, srcaddr, srcpart, dstaddr, sizeof(*value)); // inverted src/dst addrs!
	if(ret<0)
		xmprintf("XM_memory_copy(%d, %x, %x, %x, %d): ret %d\n",
			dstpart, dstaddr, srcpart, srcaddr, sizeof(*value), ret);
	XDump(value, (void*)dstaddr, sizeof(*value));
	return 0;
}

static int CheckMemArea(xm_u32_t startAddr, xm_u32_t size)
{
    int i, ret, nr=8;
	struct xmcMemoryArea memAreas[nr];

	ret=XM_get_physmem_map(memAreas, nr);
    if(ret != nr)
        return -1;
    for (i=0; i < nr; i++){
        if (memAreas[i].startAddr == startAddr && memAreas[i].size >= size)
            break;
    }

    if(i >= nr)
        return -1;

    return i;
}

int CmdMemWFile(char *line)
{
    int narg;
    char *arg[3];
    int srcpart;
    int dstpart;
    xm_u32_t srcaddr;
    xm_u32_t dstaddr;
    int i, n, fd, ret, fsize;

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg == -1){
        xmprintf("Error: dstpart not set\n");
        return -1;
    }else if(narg == 0){
        xmprintf("Error: dstaddr not set\n");
        return -1;
    }else if(narg == 1){
        xmprintf("Error: file not set\n");
        return -1;
    }

    dstpart = atoi(arg[0]);
    sscanf(arg[1], "%x", &dstaddr);
    
    fd = open(arg[2], O_RDONLY);
    if (fd < 0){
        xmprintf("Error: invalid file %s\n", arg[2]);
        return -1;
    }

    fsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // TBD check XM API, ABI and signature
    if (CheckMemArea(dstaddr, fsize) < 0){
        xmprintf("Error: invalid dstaddr %x\n", dstaddr);
        return -1;
    }
 
    srcpart = XM_PARTITION_SELF;
    srcaddr = Va2Pa(buf);

    if(!dstaddr || !srcaddr)
        return -1;

    n = 0;
    while((i = read(fd, buf, sizeof(buf))) > 0)
    {
        ret=XM_memory_copy(dstpart, srcaddr, srcpart, dstaddr+n, i);
        if(ret<0)
            xmprintf("XM_memory_copy(%d, %x, %x, %x, %d): ret %d\n",
                    dstpart, srcaddr, srcpart, dstaddr+n, i, ret);
        /*XDump(buf, (void*)dstaddr, i);*/
        n += ret;
    }

    return 0;
}
#endif


int CmdMemAreas(char *line) {
    int i, j, ret, nr;
#ifdef linux
    nr = 7;
#else
    extern partitionInformationTable_t partitionInformationTable;
    partitionInformationTable_t *PIT=&partitionInformationTable;
    nr = PIT->noPhysicalMemoryAreas;
#endif
    if (line)
        nr = atoi(line);
	struct xmcMemoryArea memAreas[nr];

	ret=XM_get_physmem_map(memAreas, nr);
	if (ret != nr)
        return ret;

    for (i=0; i < nr; i++){
            xmprintf("Area id:%d start: 0x%08x size: %08d flags:0x%08x \n", i,
                memAreas[i].startAddr, memAreas[i].size, memAreas[i].flags);
            for(j=0; j<7; j++){
                if(memAreas[i].flags & 1<<j)
                    xmprintf(" %s", MemFlagToStr(memAreas[i].flags & 1<<j));
            }
            xmprintf("\n");
    }

    return ret;
}

int CmdXmHypervisor(char *line){
    int ret;
    xmSystemStatus_t status;

#ifdef sparcv8
    ret=XM_get_system_status(&status);
#else
    ret=XM_system_get_status(&status);
#endif
    
    if(ret<0)
        return ret;

    xmprintf("resetCounter %lld noHmEvents: %lld noIrqs: %d currentMaf: %lld resetCounter: %d\n"
            "noQueuingPortMsgsReceived %lld noSamplingPortMsgRead: %lld\n"
            "noSamplingPortMsgsWritten %lld noQueuingPortMsgsSent %lld\n\n",
            status.noHmEvents, status.noIrqs, status.currentMaf, status.resetCounter, 
            status.noQueuingPortMsgsReceived, status.noSamplingPortMsgsRead,
            status.noSamplingPortMsgsWritten, status.noQueuingPortMsgsSent);
    return ret;
}

int CmdXmPlan(char *line){
    int narg, ret, planid=0;
    xmPlanStatus_t status;
    char *arg[1];

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg < 0){
        XM_get_plan_status(&status);
        xmprintf("plan next: %ld current: %ld prev: %ld switchTime: %lld\n",
                status.next, status.current, status.prev, status.switchTime);
        return 0;
    }

    planid = atoi(arg[0]);
    ret=XM_set_plan(planid);
    return ret;
}

int CmdWriteConsole(char *line){
    int narg, ret;
    char *arg[1];

    narg=SplitLine(line, arg, NELEM(arg), 1);
    if(narg < 0)
        return 0;

    ret=XM_write_console(arg[0], strlen(arg[0]));
    return ret;
}
