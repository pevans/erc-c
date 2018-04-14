/*
 * apple2_debug.c
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "apple2/apple2.h"
#include "apple2/debug.h"
#include "apple2/hires.h"
#include "mos6502/dis.h"
#include "mos6502/mos6502.h"
#include "vm_di.h"
#include "vm_event.h"

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

/*
 * A table of commands that we support in the debugger. This list is
 * printed out (in somewhat readable form) by the help/h command.
 */
apple2_debug_cmd cmdtable[] = {
    { "break", "b", apple2_debug_cmd_break, 1, "<addr>",
        "Add breakpoint at <addr>", },
    { "dblock", "db", apple2_debug_cmd_dblock, 2, "<from> <to>",
        "Disassemble a block of code", },
    { "help", "h", apple2_debug_cmd_help, 0, "",
        "Print out this list of commands", },
    { "hdump", "hd", apple2_debug_cmd_hdump, 2, "<from> <to>",
        "Hex dump memory in a given region", },
    { "hidump", "hid", apple2_debug_cmd_hidump, 1, "<file>",
        "Dump hires graphics memory to file", },
    { "jump", "j", apple2_debug_cmd_jump, 1, "<addr>",
        "Jump to <addr> for next execution", },
    { "printaddr", "pa", apple2_debug_cmd_printaddr, 1, "<addr>",
        "Print the value at memory address <addr>", },
    { "printstate", "ps", apple2_debug_cmd_printstate, 0, "",
        "Print the machine and CPU state", },
    { "quit", "q", apple2_debug_cmd_quit, 0, "",
        "Quit the emulator", },
    { "resume", "r", apple2_debug_cmd_resume, 0, "",
        "Resume execution", },
    { "step", "s", apple2_debug_cmd_step, 0, "",
        "Execute the current opcode and break at the next", },
    { "unbreak", "u", apple2_debug_cmd_unbreak, 1, "<addr>",
        "Remove breakpoint at <addr>", },
    { "writeaddr", "wa", apple2_debug_cmd_writeaddr, 2, "<addr> <byte>",
        "Write <byte> at <addr>", },
    { "writestate", "ws", apple2_debug_cmd_writestate, 2, "<reg> <byte>",
        "Write <byte> into <reg>", },
};

#define CMDTABLE_SIZE (sizeof(cmdtable) / sizeof(apple2_debug_cmd))

/*
 * Return the next argument in a string passed in for input with the
 * debugger. All arguments are space-separated.
 */
char *
apple2_debug_next_arg(char **str)
{
    char *tok;

    while ((tok = strsep(str, " "))) {
        if (*tok == '\0') {
            continue;
        }

        break;
    }

    return tok;
}

/*
 * Given a string (str), parse it into a hexadecimal value. This
 * function returns -1 if the string is invalid or was NULL to begin
 * with.
 */
int
apple2_debug_addr(const char *str)
{
    int addr;

    if (str == NULL) {
        return -1;
    }

    // We do accept the $XX notation for hexadecimals; it's easy to do,
    // since we can just skip past that part of the string.
    if (*str == '$') {
        str++;
    }

    addr = strtol(str, NULL, 16);
    if (addr == 0 && errno == EINVAL) {
        return -1;
    }

    return addr;
}

/*
 * Add a breakpoint for addr
 */
void
apple2_debug_break(int addr)
{
    if (addr < 0 || addr >= BREAKPOINTS_MAX) {
        return;
    }

    breakpoints[addr] = true;
}

/*
 * Remove a breakpoint for addr, if one is set
 */
void
apple2_debug_unbreak(int addr)
{
    if (addr < 0 || addr >= BREAKPOINTS_MAX) {
        return;
    }

    breakpoints[addr] = false;
}

/*
 * Return true if there is a breakpoint set for addr
 */
bool
apple2_debug_broke(int addr)
{
    if (addr < 0 || addr >= BREAKPOINTS_MAX) {
        return false;
    }

    return breakpoints[addr];
}

/*
 * Remove all breakpoints that have been set for any address. This
 * function doesn't have a ton of functional value--it's main use is in
 * testing, when we destroy all breakpoints in the singleton breakpoint
 * array when tearing down for a given test.
 */
void
apple2_debug_unbreak_all()
{
    memset(breakpoints, false, BREAKPOINTS_MAX);
}

/*
 * Print a debugger prompt and return a newly-allocated string
 * containing the contents of the input given from the user. (You
 * _must_ free the string that is returned when you are done with it!
 * Unless you're ok with memory leaks!)
 */
char *
apple2_debug_prompt()
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

/*
 * Given a string (str), execute the command with any arguments given to
 * it (assuming they are space-separated).
 */
void
apple2_debug_execute(const char *str)
{
    char *tok, *ebuf, *orig;
    apple2_debug_cmd *cmd;
    apple2_debug_args args;

    orig = ebuf = strdup(str);
    cmd = NULL;

    tok = apple2_debug_next_arg(&ebuf);

    // No input
    if (tok == NULL) {
        free(orig);
        return;
    }

    cmd = apple2_debug_find_cmd(tok);

    // No command found
    if (cmd == NULL) {
        free(orig);
        return;
    }

    args.addr1 = 0;
    args.addr2 = 0;
    args.target = NULL;

    switch (cmd->nargs) {
        case 2:
            args.target = apple2_debug_next_arg(&ebuf);

            // This _may_ be -1 if we have a string target for argument
            // 1, as in the writestate command
            args.addr1 = apple2_debug_addr(args.target);

            args.addr2 = apple2_debug_addr(apple2_debug_next_arg(&ebuf));

            // But if this is -1, then something went wrong
            if (args.addr2 == -1) {
                free(orig);
                return;
            }

            break;

        case 1:
            args.target = apple2_debug_next_arg(&ebuf);
            args.addr1 = apple2_debug_addr(args.target);

            break;

        case 0:
        default:
            break;
    }

    cmd->handler(&args);

    free(orig);
}

/*
 * Compare a string key (k) with a apple2_debug_cmd (elem) name or abbrev
 * field. This is the function we use for bsearch() in
 * apple2_debug_find_cmd().
 */
static int
cmd_compar(const void *k, const void *elem)
{
    const char *key = (const char *)k;
    const apple2_debug_cmd *cmd = (const apple2_debug_cmd *)elem;

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
apple2_debug_cmd *
apple2_debug_find_cmd(const char *str)
{
    return (apple2_debug_cmd *)bsearch(str, &cmdtable, CMDTABLE_SIZE,
                                   sizeof(apple2_debug_cmd), cmd_compar);
}

/*
 * Add a breakpoint at the address given in args->addr1
 */
DEBUG_CMD(break)
{
    apple2_debug_break(args->addr1);
}

/*
 * Print a list of the commands we support (including this one!)
 */
DEBUG_CMD(help)
{
    FILE *stream = (FILE *)vm_di_get(VM_OUTPUT);
    apple2_debug_cmd *cmd;
    
    for (int i = 0; i < CMDTABLE_SIZE; i++) {
        cmd = &cmdtable[i];
        fprintf(stream, "%-15s%-5s%-15s%s\n", cmd->name, cmd->abbrev, 
                cmd->argdesc, cmd->desc);
    }
}

/*
 * Remove any breakpoint at the current PC and unpause execution
 */
DEBUG_CMD(resume)
{
    apple2 *mach = (apple2 *)vm_di_get(VM_MACHINE);

    // If we paused because of a breakpoint, then we need to clear it
    // before we can really keep moving.
    apple2_debug_unbreak(mach->cpu->PC);

    mach->paused = false;
    mach->debug = false;
}

/*
 * Print the state contents of the CPU and machine
 */
DEBUG_CMD(printstate)
{
    apple2 *mach = (apple2 *)vm_di_get(VM_MACHINE);
    mos6502 *cpu = mach->cpu;
    FILE *out = (FILE *)vm_di_get(VM_OUTPUT);

    fprintf(out, "CPU:  A:%02x X:%02x Y:%02x P:%02x S:%02x PC:%04x\n",
            cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, cpu->PC);

    fprintf(out, "MACH: BS:%02x CM:%02x DM:%02x MM:%02x STROBE:%02x\n",
            mach->bank_switch, mach->color_mode, mach->display_mode,
            mach->memory_mode, mach->strobe);
}

/*
 * Print the value at the address given in args->addr1
 */
DEBUG_CMD(printaddr)
{
    // FIXME: This is... too machine-specific; we need to abstract this logic
    
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    FILE *stream = (FILE *)vm_di_get(VM_OUTPUT);

    fprintf(stream, "$%02X\n", mos6502_get(cpu, args->addr1));
}

/*
 * Jump from the current PC to one given in args->addr1
 */
DEBUG_CMD(jump)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    cpu->PC = args->addr1;
}

/*
 * Write the byte value given in args->addr2 at the address given in
 * args->addr1.
 */
DEBUG_CMD(writeaddr)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    mos6502_set(cpu, args->addr1, args->addr2);
}

/*
 * Change the machine state given in args->target to the value in
 * args->addr2. Right now, you can only change CPU registers, but
 * ultimately it'd be nice to change other things like strobe, display
 * mode, etc.
 */
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

/*
 * Quit the program entirely
 */
DEBUG_CMD(quit)
{
    exit(0);
}

/*
 * Remove the breakpoint at the given address
 */
DEBUG_CMD(unbreak)
{
    apple2_debug_unbreak(args->addr1);
}

/*
 * Execute the current opcode at PC in the CPU, then break at the
 * next opcode.
 */
DEBUG_CMD(step)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);

    apple2_debug_unbreak(cpu->PC);
    mos6502_execute(cpu);
    apple2_debug_break(cpu->PC);
}

/*
 * Disassemble a block of memory from one given address to another.
 * Useful when you want to see what memory looks like in a given region.
 * Note that the disassembler is a bit dumb, and can't tell what data
 * are opcodes is meant to be standalone binary data (the sort you would
 * get if you used "DFB" notation in an assembler).
 */
DEBUG_CMD(dblock)
{
    if (args->addr1 > args->addr2) {
        return;
    }

    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    FILE *stream = log_stream();

    mos6502_dis_scan(cpu, stream, args->addr1, args->addr2);
}

/*
 * Print a hex dump of a region of memory
 */
DEBUG_CMD(hdump)
{
    if (args->addr1 > args->addr2) {
        return;
    }

    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    FILE *stream = log_stream();

    vm_segment_hexdump(cpu->rmem, stream, args->addr1, args->addr2);
}

DEBUG_CMD(hidump)
{
    FILE *stream;

    stream = fopen(args->target, "w");
    if (stream == NULL) {
        return;
    }

    apple2 *mach = (apple2 *)vm_di_get(VM_MACHINE);
    apple2_hires_dump(mach, stream);

    fclose(stream);
}
