#include <criterion/criterion.h>

#include <stdio.h>
#include <stdlib.h>

#include "apple2/apple2.h"
#include "apple2/debug.h"
#include "apple2/event.h"
#include "vm_di.h"

static char buf[BUFSIZ];
static FILE *stream = NULL;
static apple2_debug_args args;

static apple2 *mach;

static void
setup()
{
    // Don't worry...we don't care about /dev/null, because we'll hijack
    // the output with setvbuf below
    stream = fopen("/dev/null", "w");
    if (stream == NULL) {
        perror("Could not open /dev/null for stream testing");
        exit(1);
    }

    vm_di_set(VM_OUTPUT, stream);
    log_open(stream);

    // Writing to stream will now write to buf
    setvbuf(stream, buf, _IOFBF, BUFSIZ);

    mach = apple2_create(100, 100);
    vm_di_set(VM_MACHINE, mach);
    vm_di_set(VM_CPU, mach->cpu);

    apple2_event_init();
}

static void
teardown()
{
    memset(buf, 0, BUFSIZ);
    fclose(stream);

    apple2_free(mach);

    apple2_debug_unbreak_all();
}

TestSuite(apple2_debug, .init = setup, .fini = teardown);

/*
 * I dunno what we'll end up printing! It'll change as we add commands,
 * so we just test if we print _something_.
 */
Test(apple2_debug, cmd_help)
{
    apple2_debug_cmd_help(&args);
    cr_assert_neq(strlen(buf), 0);
}

Test(apple2_debug, execute)
{
    apple2_debug_execute("help");
    cr_assert_neq(strlen(buf), 0);
}

Test(apple2_debug, next_arg)
{
    char *orig, *str = strdup("ok    hey");
    orig = str;

    cr_assert_str_eq(apple2_debug_next_arg(&str), "ok");
    cr_assert_str_eq(apple2_debug_next_arg(&str), "hey");

    free(orig);
}

/*
 * Right now, there's not a great way of testing apple2_debug_prompt() since
 * it's dependent on stdin. Or, maybe there is with a hack, but I'd like
 * to do something more appropriate in the future.
 *
 * Test(apple2_debug, prompt)
 */

Test(apple2_debug, addr)
{
    cr_assert_eq(apple2_debug_addr("faaa"), 0xfaaa);
    cr_assert_eq(apple2_debug_addr("123"), 0x123);
    cr_assert_eq(apple2_debug_addr("$34"), 0x34);
    cr_assert_eq(apple2_debug_addr("hey"), -1);
}

/* 
 * The find_cmd test also tests cmd_compar--at least, indirectly; that
 * function is statically declared, and we don't have access to test it
 * here.
 *
 * Test(apple2_debug, cmd_compar) 
 */
Test(apple2_debug, find_cmd)
{
    apple2_debug_cmd *cmd;

    // Can we find anything?
    cr_assert_neq(apple2_debug_find_cmd("resume"), NULL);

    // Make sure NULL is returned for bad commands
    cr_assert_eq(apple2_debug_find_cmd("THIS_DOESNT_EXIST"), NULL);

    // Find a second command...
    cmd = apple2_debug_find_cmd("help");
    cr_assert_neq(cmd, NULL);

    // and see if we can find the short version
    cr_assert_eq(apple2_debug_find_cmd("h"), cmd);
}

Test(apple2_debug, cmd_resume)
{
    mach->paused = true;
    apple2_debug_cmd_resume(&args);

    cr_assert_eq(mach->paused, false);
}

Test(apple2_debug, cmd_printstate)
{
    apple2_debug_cmd_printstate(&args);
    cr_assert_neq(strlen(buf), 0);
}

Test(apple2_debug, cmd_printaddr)
{
    args.addr1 = 123;
    mos6502_set(mach->cpu, args.addr1, 127);
    apple2_debug_cmd_printaddr(&args);
    cr_assert_str_eq(buf, "$7F\n");
}

Test(apple2_debug, cmd_jump)
{
    args.addr1 = 123;
    apple2_debug_cmd_jump(&args);
    cr_assert_eq(mach->cpu->PC, 123);
}

Test(apple2_debug, cmd_writeaddr)
{
    args.addr1 = 123;
    args.addr2 = 0xf5;

    apple2_debug_cmd_writeaddr(&args);
    cr_assert_eq(mos6502_get(mach->cpu, args.addr1), args.addr2);
}

Test(apple2_debug, cmd_writestate)
{
    args.addr1 = 111;
    args.target = "a";
    apple2_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->A, args.addr2);

    args.addr1 = 112;
    args.target = "x";
    apple2_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->X, args.addr2);

    args.addr1 = 113;
    args.target = "y";
    apple2_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->Y, args.addr2);

    args.addr1 = 114;
    args.target = "p";
    apple2_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->P, args.addr2);

    args.addr1 = 115;
    args.target = "s";
    apple2_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->S, args.addr2);
}

Test(apple2_debug, break)
{
    apple2_debug_break(0x2);

    mos6502_set(mach->cpu, 0, 0xEA);
    mos6502_set(mach->cpu, 1, 0xEA);
    mos6502_set(mach->cpu, 2, 0xEA);
    mos6502_set(mach->cpu, 3, 0xEA);

    if (!apple2_debug_broke(mach->cpu->PC)) {
        mos6502_execute(mach->cpu);
    }

    cr_assert_eq(mach->cpu->PC, 1);

    if (!apple2_debug_broke(mach->cpu->PC)) {
        mos6502_execute(mach->cpu);
    }

    cr_assert_eq(mach->cpu->PC, 2);

    if (!apple2_debug_broke(mach->cpu->PC)) {
        mos6502_execute(mach->cpu);
    }

    cr_assert_eq(mach->cpu->PC, 2);
}

Test(apple2_debug, broke)
{
    cr_assert_eq(apple2_debug_broke(0x23), false);
    apple2_debug_break(0x23);
    cr_assert_eq(apple2_debug_broke(0x23), true);
}

Test(apple2_debug, unbreak)
{
    apple2_debug_break(0x23);

    cr_assert_eq(apple2_debug_broke(0x23), true);
    apple2_debug_unbreak(0x23);
    cr_assert_eq(apple2_debug_broke(0x23), false);
}

Test(apple2_debug, cmd_break)
{
    args.addr1 = 123;
    apple2_debug_cmd_break(&args);

    cr_assert_eq(apple2_debug_broke(123), true);
}

Test(apple2_debug, cmd_unbreak)
{
    args.addr1 = 123;
    apple2_debug_cmd_break(&args);
    cr_assert_eq(apple2_debug_broke(123), true);
    apple2_debug_cmd_unbreak(&args);
    cr_assert_eq(apple2_debug_broke(123), false);
}

Test(apple2_debug, unbreak_all)
{
    apple2_debug_break(55555);
    apple2_debug_unbreak_all();
    cr_assert_eq(apple2_debug_broke(55555), false);
}

Test(apple2_debug, cmd_step)
{
    // Just setting us up with some NOPs for the testing
    mos6502_set(mach->cpu, 0, 0xEA);
    mos6502_set(mach->cpu, 1, 0xEA);
    mos6502_set(mach->cpu, 2, 0xEA);
    mos6502_set(mach->cpu, 3, 0xEA);

    apple2_debug_break(1);
    mos6502_execute(mach->cpu);
    cr_assert_eq(mach->cpu->PC, 1);

    // We should go nowhere here
    mos6502_execute(mach->cpu);
    cr_assert_eq(mach->cpu->PC, 1);

    // Step should a) unbreak at PC, b) execute at PC
    apple2_debug_cmd_step(&args);
    cr_assert_eq(mach->cpu->PC, 2);
}

/* Test(apple2_debug, cmd_quit) */

Test(apple2_debug, cmd_disassemble)
{
    cr_assert_eq(mach->disasm, false);
    apple2_debug_cmd_disasm(&args);
    cr_assert_neq(strlen(buf), 0);
    cr_assert_eq(mach->disasm, true);
    apple2_debug_cmd_disasm(&args);
    cr_assert_eq(mach->disasm, false);
}

Test(apple2_debug, cmd_dblock)
{
    mos6502_set(mach->cpu, 0, 0xEA);
    args.addr1 = 0;
    args.addr2 = 1;
    apple2_debug_cmd_dblock(&args);
    cr_assert_neq(strlen(buf), 0);
}

Test(apple2_debug, cmd_hdump)
{
    for (int i = 0; i < 16; i++) {
        mos6502_set(mach->cpu, i, i + 0x30);
    }

    args.addr1 = 0;
    args.addr2 = 32;

    apple2_debug_cmd_hdump(&args);

    cr_assert_neq(strlen(buf), 0);
}
