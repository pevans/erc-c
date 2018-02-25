/*
 * vm_debug.c
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "mos6502.h"
#include "vm_debug.h"
#include "vm_di.h"
#include "vm_reflect.h"

/*
 * The largest address size we can set a breakpoint for
 */
#define BREAKPOINTS_MAX 0x10000

/*
 * A table of breakpoints, arranged by address in a CPU. If
 * breakpoints[i] is false, then there is no breakpoint. If it is
 * true, then there is a breakpoint at address i.
 */
static bool breakpoints[BREAKPOINTS_MAX];

vm_debug_cmd cmdtable[] = {
    { "break", "b", vm_debug_cmd_break, 1, "<addr>",
        "Add breakpoint at <addr>", },
    { "help", "h", vm_debug_cmd_help, 0, "",
        "Print out this list of commands", },
    { "jump", "j", vm_debug_cmd_jump, 1, "<addr>",
        "Jump to <addr> for next execution", },
    { "printaddr", "pa", vm_debug_cmd_printaddr, 1, "<addr>",
        "Print the value at memory address <addr>", },
    { "printstate", "ps", vm_debug_cmd_printstate, 0, "",
        "Print the machine and CPU state", },
    { "quit", "q", vm_debug_cmd_quit, 0, "",
        "Quit the emulator", },
    { "resume", "r", vm_debug_cmd_resume, 0, "",
        "Resume execution", },
    { "step", "s", vm_debug_cmd_step, 0, "",
        "Execute the current opcode and break at the next", },
    { "unbreak", "u", vm_debug_cmd_unbreak, 1, "<addr>",
        "Remove breakpoint at <addr>", },
    { "writeaddr", "wa", vm_debug_cmd_writeaddr, 2, "<addr> <byte>",
        "Write <byte> at <addr>", },
    { "writestate", "ws", vm_debug_cmd_writestate, 2, "<reg> <byte>",
        "Write <byte> into <reg>", },
};

#define CMDTABLE_SIZE (sizeof(cmdtable) / sizeof(vm_debug_cmd))

char *
vm_debug_next_arg(char **str)
{
    char *tok;

    while ((tok = strsep(str, " "))){
        if (tok == NULL) {
            return NULL;
        }

        if (*tok == '\0') {
            continue;
        }

        break;
    }

    return tok;
}

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
vm_debug_break(int addr)
{
    if (addr < 0 || addr >= BREAKPOINTS_MAX) {
        return;
    }

    breakpoints[addr] = true;
}

void
vm_debug_unbreak(int addr)
{
    if (addr < 0 || addr >= BREAKPOINTS_MAX) {
        return;
    }

    breakpoints[addr] = false;
}

bool
vm_debug_broke(int addr)
{
    if (addr < 0 || addr >= BREAKPOINTS_MAX) {
        return false;
    }

    return breakpoints[addr];
}

void
vm_debug_unbreak_all()
{
    memset(breakpoints, false, BREAKPOINTS_MAX);
}

char *
vm_debug_prompt()
{
    char buf[256];
    FILE *stream = (FILE *)vm_di_get(VM_OUTPUT);

    if (feof(stdin)) {
        return NULL;
    }

    fputs("erc> ", stream);

    if (fgets(buf, 256, stdin) == NULL) {
        return NULL;
    }

    // Cut off the newline character, if there is one
    buf[strlen(buf)-1] = '\0';

    return strdup(buf);
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

    free(ebuf);
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

DEBUG_CMD(break)
{
    vm_debug_break(args->addr1);
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
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);

    // If we paused because of a breakpoint, then we need to clear it
    // before we can really keep moving.
    vm_debug_unbreak(cpu->PC);

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

DEBUG_CMD(writestate)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);

    switch (tolower(*args->target)) {
        case 'a': cpu->A = args->addr2; break;
        case 'p': cpu->P = args->addr2; break;
        case 's': cpu->S = args->addr2; break;
        case 'x': cpu->X = args->addr2; break;
        case 'y': cpu->Y = args->addr2; break;
    }
}

DEBUG_CMD(quit)
{
    exit(0);
}

DEBUG_CMD(unbreak)
{
    vm_debug_unbreak(args->addr1);
}

DEBUG_CMD(step)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);

    vm_debug_unbreak(cpu->PC);
    mos6502_execute(cpu);
    vm_debug_break(cpu->PC);
}
