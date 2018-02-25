/*
 * vm_debug.c
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "mos6502.h"
#include "vm_debug.h"
#include "vm_di.h"
#include "vm_reflect.h"

vm_debug_cmd cmdtable[] = {
    { "help", "h", vm_debug_cmd_help, 0, "",
        "Print out this list of commands", },
    { "jump", "j", vm_debug_cmd_jump, 1, "<addr>",
        "Jump to <addr> for next execution", },
    { "printaddr", "pa", vm_debug_cmd_printaddr, 1, "<addr>",
        "Print the value at memory address <addr>", },
    { "printstate", "ps", vm_debug_cmd_printstate, 0, "",
        "Print the machine and CPU state", },
    { "resume", "r", vm_debug_cmd_resume, 0, "",
        "Resume execution", },
};

#define CMDTABLE_SIZE (sizeof(cmdtable) / sizeof(vm_debug_cmd))

char *
vm_debug_next_arg(char **str)
{
    char *tok;

    while ((tok = strsep(str, " "))) {
        if (tok == NULL) {
            return NULL;
        }

        if (*tok == '\0') {
            continue;
        }

        break;
    }

    return tok; }

int
vm_debug_addr(const char *str)
{
    int addr;

    if (str == NULL) {
        return -1;
    }

    addr = strtol(str, NULL, 16);
    if (addr == 0 && errno == EINVAL) {
        return -1;
    }

    return addr;
}

void
vm_debug_execute(const char *str)
{
    char *tok, *ebuf;
    vm_debug_cmd *cmd;
    vm_debug_args args;

    ebuf = strdup(str);
    cmd = NULL;

    tok = vm_debug_next_arg(&ebuf);

    // No input
    if (tok == NULL) {
        return;
    }

    cmd = vm_debug_find_cmd(tok);

    // No command found
    if (cmd == NULL) {
        return;
    }

    args.addr1 = 0;
    args.addr2 = 0;
    args.target = NULL;

    switch (cmd->nargs) {
        case 2:
            args.target = vm_debug_next_arg(&ebuf);

            // This _may_ be -1 if we have a string target for argument
            // 1, as in the writestate command
            args.addr1 = vm_debug_addr(args.target);

            args.addr2 = vm_debug_addr(vm_debug_next_arg(&ebuf));

            // But if this is -1, then something went wrong
            if (args.addr2 == -1) {
                return;
            }

            break;

        case 1:
            args.addr1 = vm_debug_addr(vm_debug_next_arg(&ebuf));

            // Oh no
            if (args.addr1 == -1) {
                return;
            }

            break;

        case 0:
        default:
            break;
    }

    cmd->handler(&args);
}

static int
cmd_compar(const void *k, const void *elem)
{
    const char *key = (const char *)k;
    const vm_debug_cmd *cmd = (const vm_debug_cmd *)elem;

    if (strlen(key) < 3) {
        return strcmp(key, cmd->abbrev);
    }

    return strcmp(key, cmd->name);
}

/*
 * Return the cmd struct for a command that matches str, which can
 * either be an abbreviation (if 1 or 2 characters) or a full name (if
 * otherwise). If no matching cmd can be found, return NULL.
 */
vm_debug_cmd *
vm_debug_find_cmd(const char *str)
{
    return (vm_debug_cmd *)bsearch(str, &cmdtable, CMDTABLE_SIZE,
                                   sizeof(vm_debug_cmd), cmd_compar);
}

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

DEBUG_CMD(resume)
{
    vm_reflect_pause(NULL);
}

DEBUG_CMD(printstate)
{
    vm_reflect_cpu_info(NULL);
    vm_reflect_machine_info(NULL);
}

DEBUG_CMD(printaddr)
{
    // FIXME: This is... too machine-specific; we need to abstract this logic
    
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    FILE *stream = (FILE *)vm_di_get(VM_OUTPUT);

    fprintf(stream, "$%02X\n", mos6502_get(cpu, args->addr1));
}

DEBUG_CMD(jump)
{
    // FIXME: same issue as for printaddr -- overall we need to refactor
    // vm_reflect quite a bit

    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    cpu->PC = args->addr1;
}

DEBUG_CMD(writeaddr)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    mos6502_set(cpu, args->addr1, args->addr2);
}
