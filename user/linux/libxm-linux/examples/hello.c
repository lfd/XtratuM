/*
 * $FILE: hello.c
 *
 * Hello World example using LibXM Linux (user version)
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
#include <errno.h>

#include "xm-linux.h"

int main(int argc, char *argv[])
{
    int ret;

    init_libxm(0, 0);

    ret = XM_write_console("Hello World!\n", 14);
    if (ret < 0) {
        printf("error: XM_write_console: %d %s\n", errno, strerror(errno));
        return -1;
    }

    printf("XM_write_console: %d\n", ret);
    return 0;
}
