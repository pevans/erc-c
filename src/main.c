#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "option.h"

/*
 * This function will establish the base environment that we want to use
 * while we execute.
 */
static void
init(int argc, char **argv)
{
    int options_ok;

    // If the option_parse() function returns zero, that means that it's
    // signaled to us that we should stop now. Whether that means we are
    // stopping in _error_ (bad input), or just because you asked for
    // --help, is not really specified. We exit with a non-zero error
    // code in any case.
    options_ok = option_parse(argc, argv);
    if (options_ok == 0) {
        const char *err = option_get_error();

        if (strlen(err) > 0) {
            fprintf(stderr, "%s\n", err);
            option_print_help();
        }

        exit(1);
    }

    // We're literally using stdout in this heavy phase of development.
    log_open(stdout);
}

/*
 * And this is the teardown function.
 */
static void
finish()
{
    // Close any file sources we had opened
    for (int i = 1; i <= OPTION_MAX_DISKS; i++) {
        FILE *stream = option_get_input(i);

        if (stream != NULL) {
            fclose(stream);
        }
    }

    log_close();
}

/*
 * This is what will run when the program begins, if you were new to how
 * C works.
 */
int
main(int argc, char **argv)
{
    init(argc, argv);

    // When we exit, we want to wrap up a few loose ends. This syscall
    // will ensure that `finish()` runs whether we return from main
    // successfully or if we run `exit()` from elsewhere in the program.
    atexit(finish);

    // ha ha ha ha #nervous #laughter
    printf("Hello, world\n");
}
