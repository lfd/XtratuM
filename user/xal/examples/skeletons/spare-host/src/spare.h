/*
 * $FILE: spare.h
 *
 * Spare Manager global header file
 *
 * Author: Jordi Sánchez, <jsanchez@ai2.upv.es>
 * Changes: on 14 Dec, 2011 by Salva Peiro, <speiro@ai2.upv.es>
 *          Fetch sampling ports parameters from (XM_CF) custom file.
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */
#ifndef _SPARE_H_
#define _SPARE_H_

#include <xm.h>

#define PARTITION_NONTRUSTED    0
#define PARTITION_TRUSTED       1
#define ALLOWED_MINIMUM         (0.01f)

#define STR(x)                  #x
#define ESTR(x)                 STR(x)
#define DPRINT(fmt,...)         if(1){ printf("["__FILE__":"ESTR(__LINE__)"] %s " fmt, __FUNCTION__, ## __VA_ARGS__); }
#define ASSERT(x)               if(!(x)){\
                                    printf(__FILE__":"ESTR(__LINE___)" Assert failed: "#x"\n");\
                                    XM_halt_partition(XM_PARTITION_SELF);\
                                }

/* Current message */
struct spareMsg {
    xm_u32_t usage;
    xm_u32_t sequence;
};

struct partitionData {
    
    xmId_t partitionId;         /* Partition Identifier */
    
    struct {                    /* Scheduling data */
        xm_u32_t schedClass;    /* Trusted / Non-trusted */
        xm_u32_t priority;
    } schedData;

    char *portName;             /* Sampling port where the partition is going to place the requests */
    xm_u32_t portSize;          /* Requests port size */
    xm_s32_t portDesc;          /* Port descriptor */
    struct spareMsg msg;        /* Spare message */
    float request;              /* Partition request data */
    float assigned;             /* Spare assigned fraction */
    xm_u32_t slot;              /* Slot time */
};

#endif
