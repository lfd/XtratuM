/*
 * $FILE: main.c
 *
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <elf.h>

extern int ProcessElf(int fdElf, int excludeSections[], int noExcludeSections);
extern int WriteXef(int fdXef, int imageId);

#define USAGE "USAGE:\nelf2xef [-o <output>] [-i <id>] [-x <section_id>] <input>\n"


int main(int argc, char **argv) {
    int fdElf, fdXef, opt, imageId=0;
    char *output, stdOutput[]="a.out";
    int noExcludeSections=0, *excludeSections=0;
    output=stdOutput;

    if (argc<2) {
	fprintf(stderr, USAGE);
	return -2;
    }

    while ((opt=getopt(argc, argv, "o:i:x:")) != -1) {
        switch (opt) {
        case 'o':
            if (!(output=malloc(strlen(optarg)+1))) {
		fprintf(stderr, "[malloc] Dynamic memory pool exhausted\n");
		return -2;
	    }
            strcpy(output, optarg);
            break;
	case 'i':
	    imageId=atoi(optarg);
            break;
	    case 'x':
            excludeSections=realloc(excludeSections, sizeof(int)*(noExcludeSections+1));
            if(!excludeSections){
		        fprintf(stderr, "[realloc] Dynamic memory pool exhausted\n");
		        return -2;
            }
            excludeSections[noExcludeSections++]=atoi(optarg);
            break;
        default: /* ? */
	    fprintf(stderr, USAGE);
	    return -2;
        }
    }
    
    if ((argc-optind)!=1) {
	fprintf(stderr, USAGE);
	return -2;
    }
    if ((fdElf=open(argv[optind], O_RDONLY))<0) {
	fprintf(stderr, "[open] Unable to open %s\n", argv[optind]);
	return -2;
    }

    if ((fdXef=open(output, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP))<0) {
	fprintf(stderr, "[open] Unable to open %s\n", output);
    }
    ProcessElf(fdElf, excludeSections, noExcludeSections);
    WriteXef(fdXef, imageId);
    return 0;
}
