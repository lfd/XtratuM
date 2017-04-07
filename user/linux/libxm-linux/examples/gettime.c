/*
 * $FILE: gettime.c
 *
 * Get_time example using LibXM Linux (user version)
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
#include <errno.h>

#include "xm-linux.h"

int main(int argc, char *argv[])
{
    long long t0, t1, t2;
    int ret;

    init_libxm(0, 0);

    ret = XM_get_time(XM_HW_CLOCK, &t0);
    ret = XM_get_time(XM_HW_CLOCK, &t1);
    sleep(1);
    ret = XM_get_time(XM_HW_CLOCK, &t2);
    if (ret < 0) {
        printf("error: XM_get_time: %d %s\n", errno, strerror(errno));
        return -1;
    }

    printf("XM_get_time: %lld %lld: %lld\n", t1, t2, (t2 - t1) - (t1 - t0));
    return 0;
}
