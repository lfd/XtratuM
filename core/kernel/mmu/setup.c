/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <assert.h>
#include <stdc.h>
#include <processor.h>
#include <sched.h>
#include <kthread.h>
#include <vmmap.h>
#include <virtmm.h>

__NOINLINE__ void FreeBootMem(void) {
    extern void IdleTask(void);
    ASSERT(!HwIsSti());
    GET_LOCAL_SCHED()->flags|=LOCAL_SCHED_ENABLED;
    Scheduling();
    IdleTask();
}
