#include <criterion/criterion.h>

#include <stdio.h>
#include <stdlib.h>

#include "apple2.h"
#include "apple2.reflect.h"
#include "vm_debug.h"
#include "vm_di.h"
#include "vm_reflect.h"

static char buf[BUFSIZ];
static FILE *stream = NULL;
static vm_debug_args args;

static apple2 *mach;
static vm_reflect *ref;

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

    // Writing to stream will now write to buf
    setvbuf(stream, buf, _IOFBF, BUFSIZ);

    ref = vm_reflect_create();
    vm_di_set(VM_REFLECT, ref);

    mach = apple2_create(100, 100);
    vm_di_set(VM_MACHINE, mach);
    vm_di_set(VM_CPU, mach->cpu);

    apple2_reflect_init();
}

static void
teardown()
{
    memset(buf, 0, BUFSIZ);
    fclose(stream);

    apple2_free(mach);
}

TestSuite(vm_debug, .init = setup, .fini = teardown);

/*
 * I dunno what we'll end up printing! It'll change as we add commands,
 * so we just test if we print _something_.
 */
Test(vm_debug, cmd_help)
{
    vm_debug_cmd_help(&args);
    cr_assert_neq(strlen(buf), 0);
}

Test(vm_debug, execute)
{
    vm_debug_execute("help");
    cr_assert_neq(strlen(buf), 0);
}

Test(vm_debug, find_cmd)
{
    vm_debug_cmd *cmd;

    // Can we find anything?
    cr_assert_neq(vm_debug_find_cmd("resume"), NULL);

    // Make sure NULL is returned for bad commands
    cr_assert_eq(vm_debug_find_cmd("THIS_DOESNT_EXIST"), NULL);

    // Find a second command...
    cmd = vm_debug_find_cmd("help");
    cr_assert_neq(cmd, NULL);

    // and see if we can find the short version
    cr_assert_eq(vm_debug_find_cmd("h"), cmd);
}

Test(vm_debug, cmd_resume)
{
    mach->paused = true;
    vm_debug_cmd_resume(&args);

    cr_assert_eq(mach->paused, false);
}

Test(vm_debug, cmd_printstate)
{
    vm_debug_cmd_printstate(&args);
    cr_assert_neq(strlen(buf), 0);
}

Test(vm_debug, cmd_printaddr)
{
    args.addr1 = 123;
    mos6502_set(mach->cpu, args.addr1, 127);
    vm_debug_cmd_printaddr(&args);
    cr_assert_str_eq(buf, "$7F\n");
}

Test(vm_debug, cmd_jump)
{
    args.addr1 = 123;
    vm_debug_cmd_jump(&args);
    cr_assert_eq(mach->cpu->PC, 123);
}

Test(vm_debug, cmd_writeaddr)
{
    args.addr1 = 123;
    args.addr2 = 0xf5;

    vm_debug_cmd_writeaddr(&args);
    cr_assert_eq(mos6502_get(mach->cpu, args.addr1), args.addr2);
}

Test(vm_debug, cmd_writestate)
{
    args.addr1 = 111;
    args.target = "a";
    vm_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->A, args.addr1);

    args.addr1 = 112;
    args.target = "x";
    vm_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->X, args.addr1);

    args.addr1 = 113;
    args.target = "y";
    vm_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->Y, args.addr1);

    args.addr1 = 114;
    args.target = "p";
    vm_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->P, args.addr1);

    args.addr1 = 115;
    args.target = "s";
    vm_debug_cmd_writestate(&args);
    cr_assert_eq(mach->cpu->S, args.addr1);
}

Test(vm_debug, break)
{
    vm_debug_break(0x2);

    mos6502_set(mach->cpu, 0, 0xEA);
    mos6502_set(mach->cpu, 1, 0xEA);
    mos6502_set(mach->cpu, 2, 0xEA);
    mos6502_set(mach->cpu, 3, 0xEA);

    mos6502_execute(mach->cpu);
    cr_assert_eq(mach->cpu->PC, 1);
    mos6502_execute(mach->cpu);
    cr_assert_eq(mach->cpu->PC, 2);
    mos6502_execute(mach->cpu);
    cr_assert_eq(mach->cpu->PC, 2);
}

Test(vm_debug, broke)
{
    cr_assert_eq(vm_debug_broke(0x23), false);
    vm_debug_break(0x23);
    cr_assert_eq(vm_debug_broke(0x23), true);
}

/* Test(vm_debug, quit) */