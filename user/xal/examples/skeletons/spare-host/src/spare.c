/*
 * $FILE: main.c
 *
 * XtratuM Spare Time Scheduler
 *
 * Author: Jordi Sánchez, <jsanchez@ai2.upv.es>
 * Changes: on 14 Dec, 2011 by Salva Peiro, <speiro@ai2.upv.es>
 *          Fetch sampling ports parameters from (XM_CF) custom file.
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xm.h>
#include "spare.h"

#define SPARE_DELTA                 1000.0f
#define SPARE_HOST_EXECUTION_TIME   200
#define MAX_SPARE_SLOTS             50
#define SPARE_ALPHA                 0.5f
#define MAX_PARTITIONS              10                  // TODO

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

struct partitionData partitionData[MAX_PARTITIONS];
struct xmcSchedSparePlan spareData;
struct xmcSchedSparePlan nextPlan;
struct xmcSchedSpareSlot spareSlots[MAX_SPARE_SLOTS];
xm_u32_t noPorts = 0;

void InitPartition(void)
{
    xm_u32_t i;
    int InitChannels(struct partitionData * partitionData);

    // Gather sampling port parameters from CF
    noPorts = InitChannels(partitionData);
    if(noPorts <= 0 || noPorts > MAX_PARTITIONS)
        printf("[Spare] Error: Failed to get sampling ports from XM_CF\n");
 
    // Open all sampling ports
    for (i=0; i<noPorts; ++i) {
        if ((partitionData[i].portDesc=XM_create_sampling_port(partitionData[i].portName, partitionData[i].portSize, XM_DESTINATION_PORT)) < 0) {
            printf("[Spare] Error: Unable to open port %s\n", partitionData[i].portName);
        }
        partitionData[i].schedData.priority = 1;
    }

    nextPlan.slotTab = spareSlots;
}

xm_s32_t AreRequestsPending(void)
{
    struct spareMsg msg;
    float request;
    xm_s32_t i, pending = 0;
    xmPartitionStatus_t state;

    /* FOR EACH PARTITION */
    for (i=0; i<noPorts; ++i) {
        if (partitionData[i].portDesc > 0) {
            XM_partition_get_status(partitionData[i].partitionId, &state);
            if (!(state.state & XM_STATUS_HALTED)) {
                if (XM_read_sampling_message(partitionData[i].portDesc, &msg, sizeof(struct spareMsg), NULL) == sizeof(struct spareMsg)) {
                    if (partitionData[i].msg.sequence != msg.sequence) {
                        partitionData[i].msg.sequence = msg.sequence;
                        request = (float)msg.usage / 1000.0f;
                        partitionData[i].request = SPARE_ALPHA*partitionData[i].request + (1.0f-SPARE_ALPHA)*request;
                        pending = 1;
                    }
                }
            }
        }
    }
    return pending;
}

void PartitionMain(void)
{
    xm_s32_t i, k, cTime;
    float nt_tot, nt_wTot, t_tot, t_wTot, minSf; //(nt) Non-trusted (t) Trusted

    InitPartition();

    printf("[Spare] Initialization done\n");

    while (1) {
        // Wait for requests
        while (!AreRequestsPending()) {
            // If no requests are pending, launch current plan and wait for the next activation
            XM_idle_self();
        }

        // Narrow the requests to the range [0..1]
        for (i=0; i<noPorts; ++i) {
            partitionData[i].request = MAX(0, partitionData[i].request);
            partitionData[i].request = MIN(1, partitionData[i].request);
        }

        // Check trusted partitions requirements
        nt_tot = nt_wTot = t_tot = t_wTot = 0;
        minSf = 10.0f;
        for (i=0; i<noPorts; ++i) {
            if (partitionData[i].schedData.schedClass == PARTITION_TRUSTED) {
                t_tot += partitionData[i].request;
                t_wTot += partitionData[i].request / partitionData[i].schedData.priority;
            } else if (partitionData[i].schedData.schedClass == PARTITION_NONTRUSTED) {
                nt_tot += partitionData[i].request;
                nt_wTot += partitionData[i].request / partitionData[i].schedData.priority;
            }
        }

        // There is spare time to assign
        if (nt_tot != 0 || t_tot != 0) {
            for (i=0; i<noPorts; ++i) {
                if (partitionData[i].schedData.schedClass == PARTITION_TRUSTED) {
                    if (t_tot >= 1.0f) {
                        partitionData[i].assigned = (partitionData[i].request / partitionData[i].schedData.priority) / t_wTot;
                    } else {
                        partitionData[i].assigned = partitionData[i].request;
                    }
                } else if (partitionData[i].schedData.schedClass == PARTITION_NONTRUSTED) {
                    if (t_tot < 1.0f && nt_wTot > 0) {
                        partitionData[i].assigned = (partitionData[i].request / partitionData[i].schedData.priority) / nt_wTot;
                        partitionData[i].assigned *= (1.0f - t_tot);
                    } else {
                        partitionData[i].assigned = 0;
                    }
                }
                if (partitionData[i].assigned < ALLOWED_MINIMUM) {
                    partitionData[i].assigned = 0;
                }
                if (0 < partitionData[i].assigned && partitionData[i].assigned < minSf) {
                    minSf = partitionData[i].assigned;
                }
            }
        }

        k = 0; cTime = 0;
        for (i=0; i<noPorts; ++i) {
            if (partitionData[i].assigned > 0) {
                nextPlan.slotTab[k].id = k;
                nextPlan.slotTab[k].partitionId = partitionData[i].partitionId;
                nextPlan.slotTab[k].duration = (xm_u32_t)((partitionData[i].assigned / minSf) * SPARE_DELTA);
                cTime += nextPlan.slotTab[k].duration;
                ++k;
            }
        }

        // Set new plan if required and launch
        if (k > 0) {
            nextPlan.header.cycleTime = cTime;
            nextPlan.header.noSlots = k;
            XM_set_spare_plan(&nextPlan);   // Set new spare plan
        } else {
            XM_set_spare_plan(NULL);        // Unset spare plan
        }
        // Launch plan
        XM_idle_self();
    }
}
