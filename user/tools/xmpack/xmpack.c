/*
 * $FILE: xmpack
 *
 * Create a pack holding the image of XM and partitions to be writen
 * into the ROM
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <features.h>
#include <xm_inc/arch/arch_types.h>
#include <xm_inc/xmef.h>

#define TOOL_NAME "xmpack"
#define DEFAULT_OUTPUT "a.out"

#define USAGE  "usage: xmpack\n" \
               "\t\tlist [-c] <package>\n" \
               "\t\textract [[-o] <file>] -c <noComponent> -f <noFile> <package>\n" \
	      "\t\tbuild [[-h|-p|-b] <file>[#<size>[MB|KB|B]][[:<file>[#<size>[MB|KB|B]]]...]]+ <package>\n"

#define DIV_ROUNDUP(a, b) ((!(a%b))?a/b:(a/b)+1)
#define ALIGN_SIZE 8
#define ALIGNTO(x) ((((~(x))+1) & (ALIGN_SIZE-1))+(x))

#define DO_REALLOC(p, s) do { \
    if (!(p=realloc(p, s))) { \
	EPrintF("Memory pool out of memory"); \
    } \
} while(0)

static struct xmefPackageHeader xmefHeader;
static xm_u8_t *xmefCompMap=0;
static xm_u32_t xmefCompMapLen=0;

static struct xmefComponent *xmefCompTab=0;
static xm_s32_t xmefNoComp=0;
static struct xmefFile *xmefFileTab=0;
static xm_s32_t xmefNoFiles=0;
static xm_s8_t *xmefStrTab=0;
static xm_s32_t xmefStrTabLen=0;

#define DO_WRITE(fd, b, s) do {\
    if (write(fd, b, s)!=s) { \
	EPrintF("Error writting package"); \
    } \
} while(0)

#if (((__BYTE_ORDER== __LITTLE_ENDIAN) && defined(CONFIG_TARGET_LITTLE_ENDIAN)) ||((__BYTE_ORDER==__BIG_ENDIAN) && defined(CONFIG_TARGET_BIG_ENDIAN)))
#define SET_BYTE_ORDER(i) (i)
#else
#define SET_BYTE_ORDER(i) ((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff)
#endif

struct fileDesc {
    xm_u32_t fileEntry;
    xm_u32_t size;
};

static void EPrintF(char *fmt, ...) {
    va_list args;
    
    fflush(stdout);
    if(TOOL_NAME != NULL)
	fprintf(stderr, "%s: ", TOOL_NAME);
  
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
  
    if (fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':')
	fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");
    exit(2); /* conventional value for failed execution */
}

static void ReadPackageFromFile(char *file) {
    int fd, r, e;
    
    if ((fd=open(file, O_RDONLY))<0)
	EPrintF("File \"%s\" cannot be opened", file);
    
    r=read(fd, (xm_u8_t *)&xmefHeader, sizeof(struct xmefPackageHeader));
    xmefHeader.signature=SET_BYTE_ORDER(xmefHeader.signature);
    xmefHeader.checksum=SET_BYTE_ORDER(xmefHeader.checksum);
    xmefHeader.version=SET_BYTE_ORDER(xmefHeader.version);
    xmefNoComp=xmefHeader.noComponents=SET_BYTE_ORDER(xmefHeader.noComponents);
    xmefHeader.componentOffset=SET_BYTE_ORDER(xmefHeader.componentOffset);
    xmefNoFiles=xmefHeader.noFiles=SET_BYTE_ORDER(xmefHeader.noFiles);
    xmefHeader.fileTabOffset=SET_BYTE_ORDER(xmefHeader.fileTabOffset);
    xmefHeader.fileDataOffset=SET_BYTE_ORDER(xmefHeader.fileDataOffset);
    xmefCompMapLen=xmefHeader.fileDataLen=SET_BYTE_ORDER(xmefHeader.fileDataLen);
    xmefStrTabLen=xmefHeader.strTabLen=SET_BYTE_ORDER(xmefHeader.strTabLen);
    xmefHeader.strTabOffset=SET_BYTE_ORDER(xmefHeader.strTabOffset);
    
    if (xmefHeader.signature!=XM_PACKAGE_SIGNATURE) {
	EPrintF("\"%s\" is not a valid package", file);
    }
    
    DO_REALLOC(xmefCompMap, xmefHeader.fileDataLen*sizeof(xm_u8_t));
    DO_REALLOC(xmefCompTab, xmefHeader.noComponents*sizeof(struct xmefComponent));
    DO_REALLOC(xmefFileTab, xmefHeader.noFiles*sizeof(struct xmefFile));
    DO_REALLOC(xmefStrTab, xmefHeader.strTabLen*sizeof(xm_s8_t));
    lseek(fd, xmefHeader.componentOffset, SEEK_SET);
    r=read(fd, (xm_u8_t *)xmefCompTab, xmefHeader.noComponents*sizeof(struct xmefComponent));
    lseek(fd, xmefHeader.fileTabOffset, SEEK_SET);
    r=read(fd, (xm_u8_t *)xmefFileTab, xmefHeader.noFiles*sizeof(struct xmefFile));
    lseek(fd, xmefHeader.strTabOffset, SEEK_SET);
    r=read(fd, (xm_u8_t *)xmefStrTab, xmefHeader.strTabLen*sizeof(xm_s8_t));
    lseek(fd, xmefHeader.fileDataOffset, SEEK_SET);
    r=read(fd, xmefCompMap, xmefHeader.fileDataLen*sizeof(xm_u8_t));

    // Process all structures' content
    for (e=0; e<xmefNoComp; e++) {
	xmefCompTab[e].flags=SET_BYTE_ORDER(xmefCompTab[e].flags);
	xmefCompTab[e].fileTabEntry=SET_BYTE_ORDER(xmefCompTab[e].fileTabEntry);
	xmefCompTab[e].noFiles=SET_BYTE_ORDER(xmefCompTab[e].noFiles);
    }
    
    for (e=0; e<xmefNoFiles; e++) {
	xmefFileTab[e].fileNameOffset=SET_BYTE_ORDER(xmefFileTab[e].fileNameOffset);
	xmefFileTab[e].fileSize=SET_BYTE_ORDER(xmefFileTab[e].fileSize);
	xmefFileTab[e].offset=SET_BYTE_ORDER(xmefFileTab[e].offset);
	xmefFileTab[e].size=SET_BYTE_ORDER(xmefFileTab[e].size);
    }
}

#define _SP "    "

static void DoList(int argc, char **argv) {
    char *file=argv[argc-1];
    xm_s32_t e, i; // j, opt;
    if (argc<3) {
	fprintf(stderr, USAGE);
	exit(0);
    }

    /* while ((opt=getopt(argc, argv, "c")) != -1) { */
    /* 	switch (opt) { */
    /* 	case 'c': */
    /* 	    showClusters=1; */
    /* 	    break; */
    /* 	default: /\* ? *\/ */
    /* 	    fprintf(stderr, USAGE); */
    /* 	    exit(2); */
    /* 	} */
    /* } */
    /* if ((optind+1)>=argc) { */
    /* 	fprintf(stderr, USAGE); */
    /* 	exit(2); */
    /* } */

    ReadPackageFromFile(file);

    fprintf(stderr, "<Package file=\"%s\" version=\"%d.%d.%d\">\n", file, XM_GET_VERSION(xmefHeader.version), XM_GET_SUBVERSION(xmefHeader.version), XM_GET_REVISION(xmefHeader.version));
    for (e=0; e<xmefHeader.noComponents; e++) {
	if (xmefCompTab[e].flags&CONFIG_COMP_HYPERVISOR_FLAG) {
	    fprintf(stderr, _SP"<XMHypervisor ");
	} else {
	    fprintf(stderr, _SP"<Partition ");
	}
	fprintf(stderr, "file=\"%s\" ", &xmefStrTab[xmefFileTab[xmefCompTab[e].fileTabEntry].fileNameOffset]);
	fprintf(stderr, "fileSize=\"%d\" ", xmefFileTab[xmefCompTab[e].fileTabEntry].fileSize);
	fprintf(stderr, "offset=\"0x%x\" ", xmefFileTab[xmefCompTab[e].fileTabEntry].offset);
	fprintf(stderr, "size=\"%d\" ", xmefFileTab[xmefCompTab[e].fileTabEntry].size);
	fprintf(stderr, ">\n");
	for (i=1; i<xmefCompTab[e].noFiles; i++) {
	    fprintf(stderr, _SP _SP"<Module ");
	    fprintf(stderr, "file=\"%s\" ", &xmefStrTab[xmefFileTab[i+xmefCompTab[e].fileTabEntry].fileNameOffset]);
	    fprintf(stderr, "size=\"%d\" ", xmefFileTab[i+xmefCompTab[e].fileTabEntry].size);
	    fprintf(stderr, "/>\n");
	}
	if (xmefCompTab[e].flags&CONFIG_COMP_HYPERVISOR_FLAG) {
	    fprintf(stderr, _SP"</XMHypervisor>\n");
	} else {
	    fprintf(stderr, _SP"</Partition>\n");
	}
    }
    fprintf(stderr, "</Package>\n");
 }

static void WritePackageToFile(char *file) {
    int fd, pos=0, e;
    if ((fd=open(file, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP))<0)
        EPrintF("File \"%s\" cannot be created", file);
    
    // Process all structures' content
    for (e=0; e<xmefNoComp; e++) {
    	xmefCompTab[e].flags=SET_BYTE_ORDER(xmefCompTab[e].flags);
    	xmefCompTab[e].fileTabEntry=SET_BYTE_ORDER(xmefCompTab[e].fileTabEntry);
    	xmefCompTab[e].noFiles=SET_BYTE_ORDER(xmefCompTab[e].noFiles);
    }

    for (e=0; e<xmefNoFiles; e++) {
    	xmefFileTab[e].fileNameOffset=SET_BYTE_ORDER(xmefFileTab[e].fileNameOffset);    	
	xmefFileTab[e].fileSize=SET_BYTE_ORDER(xmefFileTab[e].fileSize);
	xmefFileTab[e].offset=SET_BYTE_ORDER(xmefFileTab[e].offset);
    	xmefFileTab[e].size=SET_BYTE_ORDER(xmefFileTab[e].size);
    }

    lseek(fd, sizeof(struct xmefPackageHeader), SEEK_SET);
    pos=sizeof(struct xmefPackageHeader);
    pos=ALIGNTO(pos);
    lseek(fd, pos, SEEK_SET);
    xmefHeader.componentOffset=SET_BYTE_ORDER(pos);
    DO_WRITE(fd, (char *)xmefCompTab, sizeof(struct xmefComponent)*xmefNoComp);
    pos+=sizeof(struct xmefComponent)*xmefNoComp;
    pos=ALIGNTO(pos);
    lseek(fd, pos, SEEK_SET);
    xmefHeader.fileTabOffset=SET_BYTE_ORDER(pos);
    DO_WRITE(fd, (char *)xmefFileTab, sizeof(struct xmefFile)*xmefNoFiles);
    pos+=sizeof(struct xmefFile)*xmefNoFiles;
    pos=ALIGNTO(pos);
    lseek(fd, pos, SEEK_SET);
    xmefHeader.strTabOffset=SET_BYTE_ORDER(pos);
    DO_WRITE(fd, (char *)xmefStrTab, sizeof(xm_s8_t)*xmefStrTabLen);
    pos+=sizeof(xm_s8_t)*xmefStrTabLen;
    pos=ALIGNTO(pos);
    lseek(fd, pos, SEEK_SET);
    xmefHeader.fileDataOffset=SET_BYTE_ORDER(pos);
    xmefHeader.fileDataLen=SET_BYTE_ORDER(xmefCompMapLen); 
    DO_WRITE(fd, xmefCompMap, sizeof(xm_u8_t)*xmefCompMapLen);
    lseek(fd, 0, SEEK_SET);
    DO_WRITE(fd, (char *)&xmefHeader, sizeof(struct xmefPackageHeader));
    close(fd);
}

static int AddComponentToPackage(struct fileDesc *fileDesc, xm_s32_t noFiles, xm_u32_t flags) {
    xm_u8_t buff[4096];
    int e, fd, r, ptr;
    xm_s32_t cComp, cFile;

    cComp=xmefNoComp;
    xmefNoComp++;
    DO_REALLOC(xmefCompTab, xmefNoComp*sizeof(struct xmefComponent));

    cFile=xmefNoFiles;
    xmefNoFiles+=noFiles;
    DO_REALLOC(xmefFileTab, xmefNoFiles*sizeof(struct xmefFile));
    
    xmefCompTab[cComp].flags=flags;
    xmefCompTab[cComp].noFiles=noFiles;
    xmefCompTab[cComp].fileTabEntry=cFile;
    for (e=0; e<noFiles; e++, cFile++) {
	xmefFileTab[cFile].fileNameOffset=fileDesc[e].fileEntry;
	if ((fd=open(&xmefStrTab[fileDesc[e].fileEntry], O_RDONLY))<0)
	    EPrintF("File \"%s\" cannot be opened", &xmefStrTab[fileDesc[e].fileEntry]);
	xmefFileTab[cFile].fileSize=lseek(fd, 0, SEEK_END);
	xmefFileTab[cFile].size=(xmefFileTab[cFile].fileSize<fileDesc[e].size)?fileDesc[e].size:xmefFileTab[cFile].fileSize;

	if (xmefFileTab[cFile].size&7)
	    xmefFileTab[cFile].size=(xmefFileTab[cFile].size&~7)+8;
	ptr=xmefCompMapLen;
	xmefFileTab[cFile].offset=xmefCompMapLen;
	xmefCompMapLen+=xmefFileTab[cFile].size;
	DO_REALLOC(xmefCompMap, xmefCompMapLen*sizeof(xm_u8_t));
	lseek(fd, 0, SEEK_SET);
	while((r=read(fd, buff, 4096))) {
	    memcpy(&xmefCompMap[ptr], buff, r);
	    ptr+=4096;
	}
	close(fd);
    }

    return 0;
}

static inline xm_u32_t SizeStr2Size(char *input) {
    float d;
    sscanf(input, "%f", &d);
    if(strcasestr(input, "mb")) {
        d*=(1024.0*1024.0);
    } else if(strcasestr(input, "kb")) {
        d*=1024.0;
    } /*else  if(strcasestr(input, "b")) {
        } */

    return (xm_u32_t)d;
}

static inline void ProcessInputFiles(char *optarg, xm_u32_t flags) {
    char *savePtr=0, *ptr, *file, *sizeStr;
    xm_s32_t e, cStr, size;
    struct fileDesc *fileDesc=0;

    for (ptr=optarg, e=0; (file=strtok_r(ptr, ":", &savePtr)); ptr=NULL, e++) {	
	sizeStr=strpbrk(file, "#");
	if (sizeStr) {
	    *sizeStr=0;
	    sizeStr++;
	    size=SizeStr2Size(sizeStr);
	} else {
	    size=0;
	}
	cStr=xmefStrTabLen;
	xmefStrTabLen+=(strlen(file)+1);
	DO_REALLOC(xmefStrTab, xmefStrTabLen*sizeof(xm_u8_t));
	strcpy(&xmefStrTab[cStr], file);
	DO_REALLOC(fileDesc, (e+1)*sizeof(struct fileDesc));
	fileDesc[e].fileEntry=cStr;
	fileDesc[e].size=size;
    }
    AddComponentToPackage(fileDesc, e, flags);
    free(fileDesc);
}

/* static void DoExtract(int argc, char **argv) { */
/*     int comp=-1, file=-1, opt, e, fd, cluster; */
/*     char *outFile=0, *fileName=argv[argc-1]; */
/*     while ((opt=getopt(argc, argv, "o:c:f:")) != -1) { */
/* 	switch (opt) { */
/* 	case 'o': */
/* 	    if (!(outFile=malloc(strlen(optarg)+1))) { */
/* 		EPrintF("Memory pool exhausted"); */
/* 		exit(-1); */
/* 	    } */
/* 	    strcpy(outFile, optarg); */
/* 	    break; */
/* 	case 'c': */
/* 	    for (e=0; e<strlen(optarg); e++) { */
/* 		if (!isdigit(optarg[e])) { */
/* 		    fprintf(stderr, USAGE); */
/* 		    exit(2); */
/* 		} */
/* 	    } */
/* 	    comp=atoi(optarg); */
/* 	    break; */
/* 	case 'f': */
/* 	    for (e=0; e<strlen(optarg); e++) { */
/* 		if (!isdigit(optarg[e])) { */
/* 		    fprintf(stderr, USAGE); */
/* 		    exit(2); */
/* 		} */
/* 	    } */
/* 	    file=atoi(optarg); */
/* 	    break; */
/* 	default: /\* ? *\/ */
/*     	    fprintf(stderr, USAGE); */
/* 	    exit(2); */
/* 	} */
/*     } */
/*     if (comp<0||file<0) { */
/* 	fprintf(stderr, USAGE); */
/* 	exit(2); */
/*     } */
/*     if ((optind+1)>=argc) { */
/* 	fprintf(stderr, USAGE); */
/* 	exit(2); */
/*     } */
/*     ReadPackageFromFile(fileName); */
/*     if (comp>=xmefHeader.noComponents) { */
/* 	fprintf(stderr, USAGE); */
/* 	exit(2); */
/*     } */
/*     if (file>=xmefCompTab[comp].noFiles) { */
/* 	fprintf(stderr, USAGE); */
/* 	exit(2); */
/*     } */
    
/*     if (!outFile) outFile=&xmefStrTab[xmefFileTab[xmefCompTab[comp].fileTabEntry+file].fileNameOffset]; */
    
/*     //WritePackageFileToFile(outFile); */
/*     if ((fd=open(outFile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP))<0) */
/* 	EPrintF("File \"%s\" cannot be created", outFile); */
    
/*     for (e=0, cluster=xmefFileTab[xmefCompTab[comp].fileTabEntry+file].firstCluster; e<DIV_ROUNDUP(xmefFileTab[xmefCompTab[comp].fileTabEntry+file].size, CONFIG_CLUSTER_SIZE); e++) { */
/* 	DO_WRITE(fd, (xm_u8_t *)xmefClusterMap[cluster].b, CONFIG_CLUSTER_SIZE); */
/*         //memcpy((xm_u8_t *)dst, (xm_u8_t *)clusterMap[cluster].b, CONFIG_CLUSTER_SIZE); */
/*         //dst+=CONFIG_CLUSTER_SIZE; */
/*         cluster=xmefClusterMap[cluster].nextCluster; */
/*     } */
/*     close(fd); */
/* } */

static void DoBuild(int argc, char **argv) {
    char *outFile=0;
    int opt;

    memset((char *)&xmefHeader, 0, sizeof(struct xmefPackageHeader));
    xmefHeader.signature=SET_BYTE_ORDER(XM_PACKAGE_SIGNATURE);
    xmefHeader.version=SET_BYTE_ORDER(XM_SET_VERSION(XMPACK_VERSION, XMPACK_SUBVERSION, XMPACK_REVISION));
    while ((opt=getopt(argc, argv, "p:b:h:")) != -1) {
	switch (opt) {
/*	case 'o':
	    if (!(outFile=malloc(strlen(optarg)+1))) {
		EPrintF("Memory pool exhausted");
		exit(-1);
	    }
	    strcpy(outFile, optarg);
	    break;*/
	case 'p':
	    ProcessInputFiles(optarg, 0);
	    break;
	case 'b':
	    ProcessInputFiles(optarg, CONFIG_COMP_LOAD_FLAG);
	    break;
	case 'h': 
	    ProcessInputFiles(optarg, CONFIG_COMP_HYPERVISOR_FLAG);
	    break;
	default: /* ? */
    	    fprintf(stderr, USAGE);
	    exit(2);
	}
    }

    if (argc!=(optind+2)) {
	fprintf(stderr, USAGE);
	exit(2);
    }

    outFile=argv[argc-1];
    xmefHeader.noComponents=SET_BYTE_ORDER(xmefNoComp);
    xmefHeader.noFiles=SET_BYTE_ORDER(xmefNoFiles);
    xmefHeader.strTabLen=SET_BYTE_ORDER(xmefStrTabLen);

    WritePackageToFile(outFile);
}

int main(int argc, char **argv) {
    if (argc<2){
	fprintf(stderr, USAGE);
	exit(2);
    }
    
    if (!strcasecmp(argv[1], "list")) {
	DoList(argc, argv);
	exit(0);
    }
    
	
    if (!strcasecmp(argv[1], "build")) {
	DoBuild(argc, argv);
	exit(0);
    }

    if (!strcasecmp(argv[1], "extract")) {
	//DoExtract(argc, argv);
	exit(0);
    }    

    if (!strcasecmp(argv[1], "replace")) {
	//DoReplace(argc, argv);
	exit(0);
    }    

    fprintf(stderr, USAGE);
    return 0;
}
