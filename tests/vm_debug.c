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
