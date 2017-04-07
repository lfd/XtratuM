/*
 * $FILE: ioctl.c
 *
 * LibXM Linux (user version)
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "xm-linux.h"

int main(int argc, char *argv[])
{
    int ret;
    LibXmInfo xi;

    memset(&xi, 0, sizeof(xi));
    init_libxm(0, 0);
    
    if(argv[1])
        xi.cmd = atoi(argv[1]);
    if(argv[2])
        xi.arg[0] = atoi(argv[2]);
    if(argv[3])
        xi.arg[1] = atoi(argv[3]);
    if(argv[4])
        xi.arg[2] = atoi(argv[4]);
    if(argv[5])
        xi.arg[4] = atoi(argv[5]);

    ret = XM_hcall(xi.cmd, xi.arg[0], xi.arg[1], xi.arg[2], xi.arg[3], xi.arg[4]);
    printf ("%d = XM_hcall(%x, %x, %x, %x, %x, %x)\n",
            ret, xi.cmd, xi.arg[0], xi.arg[1], xi.arg[2], xi.arg[3], xi.arg[4]);
    return ret;
}
