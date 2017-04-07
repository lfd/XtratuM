#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "xm-linux.h"

/* 
 * TODO
 * static partitionInformationTable_t *XM_params_get_PIT(void){ return; }
 * static partitionControlTable_t XM_params_get_PCT(void){return;}
 */

static LibXmInfo _xi;

__stdcall void init_libxm(partitionControlTable_t *partCtrlTab, partitionInformationTable_t *partInfTab)
{
    int partid;

    memset(&_xi, 0, sizeof(struct LibXmInfo));

    _xi.fd = open(XMDEV, O_RDWR);
    if (_xi.fd < 0) {
        fprintf(stderr, "error: init_libxm  open %s: %s\n", XMDEV, strerror(errno));
        exit(_xi.fd);
    }
    
    partid=XM_PARTITION_SELF;
    if(partid<0){
        fprintf(stderr, "error: init_libxm invalid partition id %d\n", partid);
        exit(_xi.fd);
    }
}

xm_s32_t XM_hcall(int cmd, xm_u32_t a0, xm_u32_t a1, xm_u32_t a2, xm_u32_t a3, xm_u32_t a4)
{
    _xi.cmd = _IO('X', cmd);
    _xi.arg[0] = a0;
    _xi.arg[1] = a1;
    _xi.arg[2] = a2;
    _xi.arg[3] = a3;
    _xi.arg[4] = a4;

    if(_xi.fd<0)
        return -1;

    /* return xtratum error code */
    return ioctl(_xi.fd, _xi.cmd, _xi.arg);
}
