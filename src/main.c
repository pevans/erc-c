#include <stdio.h>
#include <stdlib.h>

#include "log.h"

/*
 * This function will establish the base environment that we want to use
 * while we execute.
 */
static void
init()
{
    // We're literally using stdout in this heavy phase of development.
    log_open(stdout);
}

/*
 * And this is the teardown function.
 */
static void
finish()
{
    log_close();
}

/*
 * This is what will run when the program begins, if you were new to how
 * C works.
 */
int
main(int argc, char **argv)
{
    init();

    // When we exit, we want to wrap up a few loose ends. This syscall
    // will ensure that `finish()` runs whether we return from main
    // successfully or if we run `exit()` from elsewhere in the program.
    atexit(finish);

    // ha ha ha ha #nervous #laughter
    printf("Hello, world\n");
}
