/*
 * $FILE: xmef.h
 *
 * XM's executable format
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_XMEF_H_
#define _XM_XMEF_H_
/* <track id="build-version-nr"> */
#define XM_SET_VERSION(_ver, _subver, _rev) ((((_ver)&0xFF)<<16)|(((_subver)&0xFF)<<8)|((_rev)&0xFF))
#define XM_GET_VERSION(_v) (((_v)>>16)&0xFF)
#define XM_GET_SUBVERSION(_v) (((_v)>>8)&0xFF)
#define XM_GET_REVISION(_v) ((_v)&0xFF)
/* </track id="build-version-nr"> */
#define XMEF_XM_MAGIC  0x24584d48 // $XMH
#define XMEF_PARTITION_MAGIC 0x24584d50 // $XMP
#define XMEF_PARTITION_HDR_MAGIC 0x24585048 // $XPH

#define XM_PACKAGE_SIGNATURE 0x24584354 // $XCT

#define CONFIG_MAX_NO_FILES 3
#define CONFIG_MAX_FILE_NAME_LENGTH 32

#ifndef __ASSEMBLY__

#ifndef _XM_KERNEL_
#include <xm_inc/guest.h>
#endif

struct xmImageHdr;

/* <track id="partitionHeader"> */
struct xmPartitionHdr {
    xm_u32_t signature;
    xmAddress_t ePoint; // partition's entry point
    struct xmImageHdr *imageHdr; // Image header pointer
    partitionControlTable_t *partitionControlTable;   // PCT
    partitionInformationTable_t *partitionInformationTable; // PIT
#ifdef CONFIG_MMU
    xmAddress_t pagTabAddr;
    xmSize_t pagTabSize;
#endif
};
/* </track id="partitionHeader"> */

//@ \void{<track id="xefCustomFile">}
struct xefCustomFile {
    xmAddress_t sAddr;
    xmSize_t size;
};

//@ \void{</track id="xefCustomFile">}
/* <track id="imageHeader"> */
struct xmImageHdr {
    xm_u32_t signature;
    xm_u32_t xmAbiVersion; // XM's abi version
    xm_u32_t xmApiVersion; // XM's api version
    xm_u32_t imageId;      // To bind this image with the configuration
    xm_u32_t checksum; // header's checksum
    xmAddress_t sAddr; // partition's start memory address
    xmAddress_t eAddr; // partition's end memory address
    union {
        xmAddress_t ePoint; // XtratuM's entry point
        struct xmPartitionHdr *defaultPartitionHdr; // or partition Hdr.
    } entry;
    xm_u32_t noModules;
    struct xefCustomFile moduleTab[CONFIG_MAX_NO_FILES];
};
/* </track id="imageHeader"> */

/*  <track id="xmefFile"> */
struct xmefFile {
    xmAddress_t fileNameOffset;
    xmSize_t fileSize;
    xmAddress_t offset;
    xmSize_t size;
};
/*  </track id="xmefFile"> */

/*  <track id="xmefPartition"> */
struct xmefComponent {
    xm_u32_t flags;
#define CONFIG_COMP_HYPERVISOR_FLAG 0x1
#define CONFIG_COMP_LOAD_FLAG 0x2
    xm_u32_t fileTabEntry;
    xm_s32_t noFiles;
};
/*  </track id="xmefPartition"> */

/*  <track id="xmefPackageHeader-vers"> */
#define XMPACK_VERSION 1
#define XMPACK_SUBVERSION 0
#define XMPACK_REVISION 0
/*  </track id="xmefPackageHeader-vers"> */

/*  <track id="xmefContainerHdr"> */
struct xmefPackageHeader {
    xm_u32_t signature;
    xm_u32_t version;
    xm_u32_t checksum;
    xm_u32_t noComponents;    // Number of components.
    xmAddress_t componentOffset;  // Offset to the table of components area.
    xm_u32_t noFiles;         // Number of files in the container.
    xmAddress_t fileTabOffset;    // Offset to the table of files.
    xmSize_t fileDataLen;         // Length of the file data area.
    xmAddress_t fileDataOffset;   // Offset to files data content.
    xmSize_t strTabLen;        // Length of the string area.
    xmAddress_t strTabOffset;  // Offset to the string area.
};
/*  </track id="xmefContainerHdr"> */

/* <track id="xefHdr"> */
struct xefHdr {
#define XEFSIGNATURE 0x24584546
    xm_u32_t signature;
    xm_u32_t imageId;      // To bind this image with its configuration
    xmAddress_t secOffset;
    xm_s32_t nSec;
    xm_s32_t memSz;
    xmAddress_t relOffset;
    xm_s32_t nRel;
    xmAddress_t entry;
};
/* </track id="xefHdr"> */

/* <track id="xefSegment"> */
struct xefSection {
    xmAddress_t pAddr;
    xmAddress_t vAddr;
    //xm_u32_t memSz;
    xm_u32_t fileSz;
    xmAddress_t offset;
};
/* </track id="xefSegment"> */

#define RELATIVE_REL 2
#define ABS_REL 1

struct xefRel {
    xm_s32_t info;
    xmAddress_t offset;
};

#endif

#endif
