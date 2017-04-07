/*
 * $FILE: objdir.h
 *
 * Object directory definition
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_OBJDIR_H_
#define _XM_OBJDIR_H_

typedef xm_u32_t xmObjDesc_t;
// Object descriptor:
// VALIDITY  | CLASS | PARTITIONID | ID
//     1     |  11   |      10     |  10

#define OBJDESC_CLASS_MASK ((1<<11)-1)
#define OBJDESC_PARTITIONID_MASK ((1<<10)-1)
#define OBJDESC_ID_MASK ((1<<10)-1)
#define OBJDESC_VALIDITY_BIT 31
#define OBJDESC_VALIDITY_MASK (1<<OBJDESC_VALIDITY_BIT)

#define OBJDESC_GET_CLASS(oD) ((oD>>20)&OBJDESC_CLASS_MASK)
#define OBJDESC_SET_CLASS(oD, class) ((oD&~(OBJDESC_CLASS_MASK<<20))|((class&OBJDESC_CLASS_MASK)<<20))

#define OBJDESC_GET_PARTITIONID(oD) ((oD>>10)&OBJDESC_PARTITIONID_MASK)
#define OBJDESC_SET_PARTITIONID(oD, partitionId) ((oD&~(OBJDESC_PARTITIONID_MASK<<10))|((partitionId&OBJDESC_PARTITIONID_MASK)<<10))

#define OBJDESC_GET_ID(oD) (oD&OBJDESC_ID_MASK)
#define OBJDESC_SET_ID(oD, id) ((oD&~(OBJDESC_ID_MASK))|((id&OBJDESC_ID_MASK)))

#define OBJDESC_IS_VALID(oD) (((oD&(OBJDESC_VALIDITY_MASK)))?0:1)
#define OBJDESC_SET_INVALID(oD) (oD|(OBJDESC_VALIDITY_MASK))
#define OBJDESC_SET_VALID(oD) (oD&~(OBJDESC_VALIDITY_MASK))

#define OBJDESC_BUILD(class, partitionId, id) (((class&OBJDESC_CLASS_MASK)<<20)|((partitionId&OBJDESC_PARTITIONID_MASK)<<10)|(id&OBJDESC_ID_MASK))

#define OBJ_CLASS_NULL 0
#define OBJ_CLASS_CONSOLE 1
#define OBJ_CLASS_TRACE 2
#define OBJ_CLASS_DIRECTORY 3
#define OBJ_CLASS_SAMPLING_PORT 4
#define OBJ_CLASS_QUEUING_PORT 5
#define OBJ_CLASS_MEM 6
#define OBJ_CLASS_HM 7
#define OBJ_CLASS_STATUS 8
#define OBJ_CLASS_UART 9
#ifdef CONFIG_SPARE_SCHEDULING
#define OBJ_CLASS_SPARE 10
#define OBJ_NO_CLASSES 11
#else
#define OBJ_NO_CLASSES 10
#endif

#ifdef _XM_KERNEL_

#include <xmconf.h>
#include <gaccess.h>

typedef xm_s32_t (*readObjOp_t)(xmObjDesc_t, void *, xmSize_t, xm_u32_t *);
typedef xm_s32_t (*writeObjOp_t)(xmObjDesc_t, void *, xmSize_t, xm_u32_t *);
typedef xm_s32_t (*seekObjOp_t)(xmObjDesc_t, xmSize_t, xm_u32_t);
typedef xm_s32_t (*ctrlObjOp_t)(xmObjDesc_t, xm_u32_t, void *);

struct object {
    readObjOp_t Read;
    writeObjOp_t Write;
    seekObjOp_t Seek;
    ctrlObjOp_t Ctrl;
};

extern const struct object *objectTab[OBJ_NO_CLASSES];

extern void SetupObjDir(void);

#define REGISTER_OBJ(_init) \
    __asm__ (".section .objsetuptab, \"a\"\n\t" \
             ".align 4\n\t" \
             ".long "#_init"\n\t" \
             ".previous\n\t")

#endif

#endif
