#include <stdio.h>
#include <stdlib.h>

#include "log.h"

static void
init()
{
    log_open();
}

static void
finish()
{
    log_close();
}

int
main(int argc, char **argv)
{
    init();

    // When we exit, we want to wrap up a few loose ends.
    atexit(finish);

	printf("Hello, world\n");
}
