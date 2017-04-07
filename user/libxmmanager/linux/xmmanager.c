/*
 * $FILE: xmmanager-linux.c
 *
 * XM Partition Manager: Linux frontend
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "xm-linux.h"
#include "xmmanager.h"

enum{
    None,
    DryRun
};

static int LinuxRead(char *line, int length){
    return read(0, line, length);
}

static int LinuxWrite(char *line, int length){
    return write(1, line, length);
}

static struct XmManagerDevice_t uartdev = {
    .flags = !DEVICE_FLAG_COOKED,

    .init = 0,
    .read = LinuxRead,
    .write = LinuxWrite,
};

int main(int argc, char *argv[]) {
    int opt, mode;

    mode = None;
    while ((opt = getopt(argc, argv, "dh")) != -1) {
        switch (opt) {
        case 'd':
            mode = DryRun;
            break;
        case 'h':
        default:
            printf("usage: xmmanager [-d] [-h]\n");
            exit(1);
        }
    }

    if (mode == None) {
        init_libxm(0, 0);
    }

    XmManager(&uartdev);
    return 0;
}
