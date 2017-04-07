/*
 * $FILE: xmmanager.h
 *
 * XM Partition Manager
 *
 * $VERSION$
 *
 * Author: Salva Peiro <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifdef ia32
#include "xm.h"
#include <std_c.h>

#elif sparcv8
#include "xm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "xm-linux.h"

#endif

#define NELEM(ary) (sizeof(ary)/sizeof(ary[0]))

int SplitLine(char *line, char *arg[], int nargs, int tokenize);
char *FindCommand(void *cmdfunc);
int GetCommandIndex(char *line, char *cmdlist[], int nelem);
int CheckPartition(int id, void *cmdfunc);
int xmprintf(char const *fmt, ...);

extern int nopart;         /* number of partitions */

static inline const char* ErrorToStr(int error)
{
    if(error>0)
        return "";

	switch(error){
	default:                    return "UNKNOWN_ERROR";

	case XM_OK:                 return "XM_OK";
	case XM_UNKNOWN_HYPERCALL:  return "XM_UNKNOWN_HYPERCALL";
	case XM_INVALID_PARAM:      return "XM_INVALID_PARAM";
	case XM_PERM_ERROR:         return "XM_PERM_ERROR";
	case XM_INVALID_CONFIG:     return "XM_INVALID_CONFIG";
	case XM_INVALID_MODE:       return "XM_INVALID_MODE";
	case XM_NOT_AVAILABLE:      return "XM_NOT_AVAILABLE";
	case XM_OP_NOT_ALLOWED:     return "XM_OP_NOT_ALLOWED";
	}
}

static inline const char *StateToStr(int state) {
    switch(state){
    default:                    return "XM_STATUS_UNKNOWN";

    case XM_STATUS_READY:       return "XM_STATUS_READY";
    case XM_STATUS_SUSPENDED:   return "XM_STATUS_SUSPENDED";
    case XM_STATUS_IDLE:        return "XM_STATUS_IDLE";
    case XM_STATUS_HALTED:      return "XM_STATUS_HALTED";
    }
}

static inline const char* TraceOpCodeToStr(xmTraceOpCode_t *traceoptcode){
    if(!traceoptcode)
        return 0;

    switch(traceoptcode->code){
    default:                        return "XM_TRACE_UNKNOWN";

    case XM_TRACE_UNRECOVERABLE:    return "XM_TRACE_UNRECOVERABLE";
    case XM_TRACE_WARNING:          return "XM_TRACE_WARNING";
    case XM_TRACE_DEBUG:            return "XM_TRACE_DEBUG";
    case XM_TRACE_NOTIFY:           return "XM_TRACE_NOTIFY";
    };
}

static inline const char* MemFlagToStr(int flag){

    switch(flag){
    default:                    return "XM_MEM_AREA_UNKNOWN";

    case XM_MEM_AREA_SHARED:    return "XM_MEM_AREA_SHARED";
    case XM_MEM_AREA_MAPPED:    return "XM_MEM_AREA_MAPPED";
    case XM_MEM_AREA_WRITE:     return "XM_MEM_AREA_WRITE";
    case XM_MEM_AREA_ROM:       return "XM_MEM_AREA_ROM";
    case XM_MEM_AREA_FLAG0:     return "XM_MEM_AREA_FLAG0";
    case XM_MEM_AREA_FLAG1:     return "XM_MEM_AREA_FLAG1";
    case XM_MEM_AREA_FLAG2:     return "XM_MEM_AREA_FLAG2";
    case XM_MEM_AREA_FLAG3:     return "XM_MEM_AREA_FLAG3";
    };
}
