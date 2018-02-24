#include <criterion/criterion.h>

#include <stdio.h>
#include <stdlib.h>
#include "vm_debug.h"
#include "vm_di.h"

static char buf[BUFSIZ];
static FILE *stream = NULL;
static vm_debug_args args;

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
}

static void
teardown()
{
    memset(buf, 0, BUFSIZ);
    fclose(stream);
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
    printf("%s", buf);
}
