/*
 * $FILE: objdir.c
 *
 * Object directory
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <brk.h>
#include <objdir.h>
#include <stdc.h>
#include <processor.h>

const struct object *objectTab[OBJ_NO_CLASSES]={[0 ... OBJ_NO_CLASSES-1] = 0};

void SetupObjDir(void) {
    extern xm_s32_t (*objectSetupTab[])(void);
    xm_u32_t e;
    for (e=0; objectSetupTab[e]; e++) {
	if (objectSetupTab[e])
	    if ((objectSetupTab[e]())<0)
		SystemPanic(0, 0, "[ObjDir] Error setting up object at 0x%x\n", objectSetupTab);
    }
}

