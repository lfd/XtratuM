/*
 * $FILE: extractinfo.c
 *
 * Extracts information from the XM binary
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define TMPFILE "/tmp/info.tmp"

#if (((__BYTE_ORDER== __LITTLE_ENDIAN) && defined(CONFIG_TARGET_LITTLE_ENDIAN)) ||((__BYTE_ORDER==__BIG_ENDIAN) && defined(CONFIG_TARGET_BIG_ENDIAN)))
#define SET_BYTE_ORDER(i) (i)
#else
#define SET_BYTE_ORDER(i) ((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff)
#endif

#define USAGE \
    "USAGE: extractinfo <xmfile>\n"

#define C_HEADER \
"/*\n" \
" * $VERSION$\n" \
" *\n" \
" * $AUTHOR$\n" \
" */\n\n" \
"#ifndef _XM_INFO_H_\n\n" \
"#define _XM_INFO_H_\n\n"

#define C_FOOT \
"#endif\n"

int main(int argc, char *argv[]) {
    char cmd[256];
    int fd, val1, val2, noElem;
    
    if (argc!=2) {
	fprintf(stderr, USAGE);
	exit(-1);
    }

    sprintf(cmd, TARGET_OBJCOPY" -O binary -j .rsv_hwirqs %s "TMPFILE, argv[1]);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
	
    fd=open(TMPFILE, O_RDONLY);
    fprintf(stderr, C_HEADER);
    fprintf(stderr, "xm_u32_t rsvHwIrqs[]={\n");
    noElem=0;
    while (read(fd, &val1, 4)>0) {
	fprintf(stderr, "    0x%x,\n", SET_BYTE_ORDER(val1));
	noElem++;
    }    
    fprintf(stderr, "};\n\n");
    fprintf(stderr, "xm_s32_t noRsvHwIrqs=%d;\n\n", noElem);
    sprintf(cmd, TARGET_OBJCOPY" -O binary -j .rsv_ioports %s "TMPFILE, argv[1]);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
    close(fd);
    fd=open(TMPFILE, O_RDONLY);
    fprintf(stderr, "struct {\n"
	    "    xm_u32_t base;\n"
	    "    xm_s32_t offset;\n"
	    "} rsvIoPorts[]={\n");
    noElem=0;
    while ((read(fd, &val1, 4)>0)&&(read(fd, &val2, 4)>0)) {
	fprintf(stderr, "    {.base=0x%x, .offset=%d,},\n", SET_BYTE_ORDER(val1), SET_BYTE_ORDER(val2));
	noElem++;
    }
    fprintf(stderr, "};\n\n");
    fprintf(stderr, "xm_s32_t noRsvIoPorts=%d;\n\n", noElem);

    sprintf(cmd, TARGET_OBJCOPY" -O binary -j .rsv_physpages %s "TMPFILE, argv[1]);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
    close(fd);
    fd=open(TMPFILE, O_RDONLY);
    fprintf(stderr, "struct {\n"
	    "    xm_u32_t address;\n"
	    "    xm_s32_t noPag;\n"
	    "} rsvPhysPages[]={\n");
    noElem=0;
    while ((read(fd, &val1, 4)>0)&&(read(fd, &val2, 4)>0)) {
	fprintf(stderr, "    {.address=0x%x, .noPag=%d,},\n", SET_BYTE_ORDER(val1), SET_BYTE_ORDER(val2));
	noElem++;
    }
    fprintf(stderr, "};\n\n");
    fprintf(stderr, "xm_s32_t noRsvPhysPages=%d;\n\n", noElem);    
    close(fd);
    unlink(TMPFILE);
    fprintf(stderr, C_FOOT);

    return 0;
}
