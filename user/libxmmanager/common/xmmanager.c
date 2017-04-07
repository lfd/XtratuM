/*
 * $FILE: xmmanager.c
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

#include "common.h"
#include "xmmanager.h"

static int CmdHelp(char *line);
static int CmdQuit(char *line);

int CmdHM(char *line);
int CmdMemAreas(char *line);
int CmdList(char *line);
int CmdPartition(char *line);
int CmdPort(char *line);
int CmdTrace(char *line);
int CmdXmHypervisor(char *line);
int CmdXDump(char *line);
int CmdXWrite(char *line);
int CmdMemWFile(char *line);
int CmdXmPlan(char *line);
int CmdWriteConsole(char *line);

int nopart = -1;         /* number of partitions */

/* cmdTab is lexicographically sorted by cmd field */
static struct CmdTab {
    int (*func) (char *);
    char cmd[64];           /* command name */
    char help[128];         /* short command description */
    char usage[128];        /* short command usage */
} cmdTab[] = {
    { CmdHelp,      "help",     "Display this help",            "help [command]" },
    { CmdHM,        "hm",       "Manage health monitoring",     "hm [read|seek|status] args" },
    { CmdList,      "list",     "List partitions",              "list [partitionid]" },
    { CmdMemAreas,  "memarea",  "List memory areas",            "memarea" },
    { CmdPartition, "partition","Control partition state",      "partition [halt|reset|resume|status|suspend] [partitionid]" },
    { CmdPort,      "port",     "Manage communication ports",   "port type operation portname portsize [message|nmessage]"},
    { CmdQuit,      "quit",     "Exit xmmanager",               "quit" },
    { CmdTrace,     "trace",    "Manage traces",                "trace [event|open|read|seek|status] traceid"},
    { CmdXDump,     "xdump",    "Dump memory contents",         "xdump [srcpart] [srcaddr] [nbytes]"},
    { CmdXWrite,    "xwrite",   "Write memory contents",        "xwrite [srcpart] [srcaddr] [value]"},
    { CmdMemWFile,  "xwfile",   "Write file contents to memory","xwfile [dstpart] [dstaddr] [file]"},
    { CmdXmHypervisor,"xm",     "Manage hypervisor status",     "xm [status]"},
    { CmdWriteConsole,"write",  "Write to the XM Console",      "write [string]"},
    { CmdXmPlan,    "plan",     "Manage hypervisor scheduling plan",       "plan [id]"},
};

static char *cmdList[NELEM(cmdTab)];

char * Trim(char *line){
    if(!line)
        return 0;
    while(line[0] == ' '){
        line++;
    }
    return line;
}

int SplitLine(char *line, char *arg[], int nargs, int tokenize) {
    int i;
    char *p;

    if(!line || !arg || !nargs)
        return -1;

    arg[0] = line;
    for (i = 0; i < nargs-1; i++) {
        arg[i]=Trim(arg[i]);
        if(!arg[i])
            break;

        arg[i+1] = strchr(arg[i], ' ');

        if (!arg[i+1])
            break;

        if(tokenize){
            p = strchr(arg[i+1], ' ');
            if(p)
                *p = '\0';
        }

        arg[i+1]++;
    }

    return i;
}

int GetCommandIndex(char *line, char *cmdlist[], int nelem){
    int i, n, narg;
    char *arg[1];
    
    narg=SplitLine(line, arg, NELEM(arg), 0);
    if(narg < 0)
            return -1;

    n=strlen(arg[0]);
    if(narg > 1)
        n=arg[1] - arg[0];

    for (i = 0; i < nelem; i++) {
        if (strncmp(cmdlist[i], arg[0], n) == 0)
            break;
    }
        
    if (i == nelem) {
        return -1;
    }
    return i;
}

char * FindCommand(void *cmdfunc){
    int i;

    for(i=0; i<NELEM(cmdTab); i++){
        if(cmdTab[i].func == cmdfunc)
            break;
    }

    if(i == NELEM(cmdTab)){
        return "unknown";
    }
    return cmdTab[i].cmd;
}

int CheckPartition(int id, void *cmdfunc) {
    char *cmd;

    cmd = FindCommand(cmdfunc);

    if (id < 0) {
        xmprintf("Error: %s: partitionId not set %d\n", cmd, id);
        return -1;
    }

    if (XM_PARTITION_SELF<0) {
        xmprintf("Error: %s: invalid XM_PARTITION_SELF %d\n", cmd, XM_PARTITION_SELF);
        return -1;
    }

    if (id == XM_PARTITION_SELF) {
        xmprintf("Error: %s: invalid partitionId equal XM_PARTITION_SELF %d\n", cmd, id);
        return -1;
    }
/*
    if (id > nopart) {
        xmprintf("Error: %s: invalid partitionId %d\n", cmd, id);
        return -1;
    }
*/
    return 0;
}

static int CmdHelp(char *line) {
    int i,j;
    int maxlen;
    
    if (line) {
        i=GetCommandIndex(line, cmdList, NELEM(cmdList));
        if (i<0) {
            xmprintf("Error: command not found: %s\n", line);
            return -1;
        }
        xmprintf("usage: %s\n", cmdTab[i].usage);
        return 0;
    }

    xmprintf("XmManager commands help:\n");
    for (i = 0; i < NELEM(cmdTab); i++) {
        xmprintf("  %s ", cmdTab[i].cmd);
        maxlen=10;
        for(j=strlen(cmdTab[i].cmd); j < maxlen; j++)
            xmprintf(" ");
        xmprintf("%s\n", cmdTab[i].help);
    }
    xmprintf("Type 'help command' for more information on a specific command.\n");
    return 0;
}

static int CmdQuit(char *line) {
    return 0;
}

/* input output */

static XmManagerDevice_t *device;

static int ReadLine(char *line, int length){
    int i;

    memset(line, '\0', length);
    xmprintf("\rxm%d> ", XM_PARTITION_SELF);
    for(i=0; i < length; i++) {
        while(device->read(&line[i], 1) != 1)
            continue;
        
        if(device->flags & DEVICE_FLAG_COOKED){
            if(i > 0 && (line[i] == '\b' || line[i] == 0x7f)){
                line[i--] = '\0';
                line[i--] = '\0';
                device->write("\b \b", 3);
                continue;
            }
            device->write(&line[i], 1);
        }

        if(line[i] == '\n' || line[i] == '\r'){
            if(device->flags & DEVICE_FLAG_COOKED)
                device->write("\n", 1);

            line[i] = '\0';
            break;
        }
    }
    return i;
}

int xmprintf(char const *fmt, ...){
	int len;
    char str[512];
	va_list args;
	
    memset(str, '\0', sizeof(str));

	va_start(args, fmt);
    len=vsprintf(str, fmt, args);
	va_end(args);

    device->write(str, len);
	return len;
}

int XmManager(XmManagerDevice_t *dev) {
    int i, n, ret;
    char *arg[2];
    char *line, buff[256];

    device = dev;
    if(device->init)
        device->init();

    for(i = 0; i < NELEM(cmdList); i++)
        cmdList[i]=cmdTab[i].cmd;
    xmprintf("XM Partition manager running on partition %d\n", XM_PARTITION_SELF);
    CmdList(0);
    
    while (1) {
        line=buff;
        n=ReadLine(line, sizeof(buff));
        if(n<=0){
            continue;
        }

        n = SplitLine(line, arg, NELEM(arg), 1);
        i = GetCommandIndex(arg[0], cmdList, NELEM(cmdList));
        if (i<0||n<0) {
            xmprintf("Error: command \"%s\" not found, type \"help\"\n", arg[0]);
            continue;
        }
        
        ret=cmdTab[i].func(arg[1]);
        xmprintf("Status: %s returned %d \"%s\"\n", (ret>=0)? "Done" : "Error", ret, ErrorToStr(ret));

        if(strncmp(cmdTab[i].cmd, "quit", 4)==0){
            break;
        }
    }

    return 0;
}
