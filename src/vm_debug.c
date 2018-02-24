/*
 * vm_debug.c
 */

#include <stdio.h>

#include "vm_debug.h"
#include "vm_di.h"

vm_debug_cmd cmdtable[] = {
    { "help", "h", 0, vm_debug_cmd_help, "",
        "Print out this list of commands", },
};

#define CMDTABLE_SIZE (sizeof(cmdtable) / sizeof(vm_debug_cmd))

DEBUG_CMD(help)
{
    FILE *stream = (FILE *)vm_di_get(VM_OUTPUT);
    vm_debug_cmd *cmd;
    
    for (int i = 0; i < CMDTABLE_SIZE; i++) {
        cmd = &cmdtable[i];
        fprintf(stream, "%-15s%-5s%-15s%s\n", cmd->name, cmd->abbrev, 
                cmd->argdesc, cmd->desc);
    }
}
