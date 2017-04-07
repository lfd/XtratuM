/*
 * $FILE: elf.c
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

#include <xm_inc/arch/arch_types.h>
#include <xm_inc/xmef.h>

#define READ(fd, buffer, size) do { \
    if (read((fd), (char *)(buffer), (size))!=(size)) { \
	fprintf(stderr, "[read] Unable to read %lu bytes\n", (unsigned long)(size));	\
	return -1; \
    } \
} while(0)

#define WRITE(fd, buffer, size) do { \
    if (write((fd), (char *)(buffer), (size))!=(size)) { \
	fprintf(stderr, "[write] Unable to write %lu bytes\n", (unsigned long)(size)); \
	return -1; \
    } \
} while(0)

static struct xefSection *xefSec;
static char **img;
static int nSec;
static struct xefRel *xefRel;
static int nRel;
static xm_u32_t entry;
static xm_s32_t memSz;

int ProcessElf(int fdElf, int excludeSections[], int noExcludeSections) {
    Elf32_Ehdr eHdr;
    Elf32_Phdr *pHdr;
    Elf32_Shdr *sHdr;
    xm_u32_t bAddr, eAddr;
    int e, i, x;

    READ(fdElf, &eHdr, sizeof(Elf32_Ehdr));

    if ((eHdr.e_ident[EI_MAG0]!=ELFMAG0)||(eHdr.e_ident[EI_MAG1]!=ELFMAG1)||(eHdr.e_ident[EI_MAG2]!=ELFMAG2)||(eHdr.e_ident[EI_MAG3]!=ELFMAG3)) {
	fprintf(stderr, "[ProcessElf] File type unknown\n");
	return -1;
    }

    if ((eHdr.e_type!=ET_EXEC)||(eHdr.e_machine!=EM_386)||(eHdr.e_phentsize!=sizeof(Elf32_Phdr))||(eHdr.e_phnum<1)||(eHdr.e_phnum>65536U/sizeof(Elf32_Phdr))) {
	fprintf(stderr, "[ProcessElf] Malformed ELF header\n");
	return -1;
    }
    
    entry=eHdr.e_entry;

    pHdr=malloc(sizeof(Elf32_Phdr)*eHdr.e_phnum);
    
    lseek(fdElf, eHdr.e_phoff, SEEK_SET);
    READ(fdElf, pHdr, sizeof(Elf32_Phdr)*eHdr.e_phnum);
    eAddr=0;
    for (bAddr=-1, e=0, xefSec=0, nSec=0, img=0, memSz=0; e<eHdr.e_phnum; e++) {
	if (pHdr[e].p_type!=PT_LOAD)
	    continue;
	if (bAddr==-1)
	    bAddr=pHdr[e].p_paddr;
	eAddr=pHdr[e].p_paddr+pHdr[e].p_memsz;
	xefSec=realloc(xefSec, sizeof(struct xefSection)*(nSec+1));
	xefSec[nSec].pAddr=pHdr[e].p_paddr;
	xefSec[nSec].vAddr=pHdr[e].p_vaddr;
	xefSec[nSec].fileSz=pHdr[e].p_filesz;
	img=realloc(img, sizeof(char **)*(nSec+1));
	img[nSec]=malloc(pHdr[e].p_filesz);
	lseek(fdElf, pHdr[e].p_offset, SEEK_SET);
	READ(fdElf, img[nSec], pHdr[e].p_filesz);
	nSec++;
    }
    memSz=eAddr-bAddr;
    sHdr=malloc(sizeof(Elf32_Shdr)*eHdr.e_shnum);
    lseek(fdElf, eHdr.e_shoff, SEEK_SET);
    READ(fdElf, sHdr, sizeof(Elf32_Shdr)*eHdr.e_shnum);
    for (e=0, xefRel=0, nRel=0; e<eHdr.e_shnum; e++) {
	if ((sHdr[e].sh_type!=SHT_REL)&&(sHdr[e].sh_type!=SHT_RELA))
	    continue;
	if (!(sHdr[sHdr[e].sh_info].sh_flags&SHF_ALLOC)) continue;
    for(x=0; x < noExcludeSections; x++){
        if(e == excludeSections[x]){
            break;
        }
    }
    if(x < noExcludeSections)
            continue;
	if (sHdr[e].sh_type==SHT_REL) {
	    Elf32_Rel *rel;
	    rel=malloc(sizeof(Elf32_Rel)*(sHdr[e].sh_size/sHdr[e].sh_entsize));
	    lseek(fdElf, sHdr[e].sh_offset, SEEK_SET);
	    READ(fdElf, rel, sizeof(Elf32_Rel)*(sHdr[e].sh_size/sHdr[e].sh_entsize));
	    xefRel=realloc(xefRel, (nRel+(sHdr[e].sh_size/sHdr[e].sh_entsize))*sizeof(struct xefRel));
	    for (i=0; i<(sHdr[e].sh_size/sHdr[e].sh_entsize); i++) {
		xefRel[nRel+i].info=ELF32_R_TYPE(rel[i].r_info);
		xefRel[nRel+i].offset=rel[i].r_offset;
		//if (ELF32_R_TYPE(rel[i].r_info)!=R_386_32) continue;
	    }
	    nRel+=(sHdr[e].sh_size/sHdr[e].sh_entsize);
	}
	
	if (sHdr[e].sh_type==SHT_RELA) {
	    fprintf(stderr, "[ProcessElf] SHT_RELA section not supported yet\n");
	    return -1;
	}
    }

    return 0;
}

int WriteXef(int fdXef, int imageId) {
    struct xefHdr xefHdr;
    int e;
    xefHdr.signature=XEFSIGNATURE;
    xefHdr.imageId=imageId;
    xefHdr.entry=entry;
    xefHdr.nSec=nSec;
    xefHdr.nRel=nRel;
    xefHdr.memSz=memSz;
    xefHdr.relOffset=lseek(fdXef, sizeof(struct xefHdr)+sizeof(struct xefSection)*nSec, SEEK_SET);
    
    WRITE(fdXef, xefRel, sizeof(struct xefRel)*nRel);

    lseek(fdXef, sizeof(struct xefHdr)+sizeof(struct xefSection)*nSec+sizeof(struct xefRel)*nRel, SEEK_SET);
    for (e=0; e<nSec; e++) {
	xefSec[e].offset=lseek(fdXef, 0, SEEK_CUR);
	WRITE(fdXef, img[e], xefSec[e].fileSz);
    }
    
    xefHdr.secOffset=lseek(fdXef, sizeof(struct xefHdr), SEEK_SET);
    WRITE(fdXef, xefSec, sizeof(struct xefSection)*nSec);
    
    lseek(fdXef, 0, SEEK_SET);
    WRITE(fdXef, &xefHdr, sizeof(struct xefHdr));
    return 0;
}
