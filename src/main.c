/*
 * main.c
 *
 * Here we define the main entry point for the program; we also define a
 * couple of functions to run when we start (init) and finish
 * (...finish).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "apple2.h"
#include "log.h"
#include "option.h"
#include "vm_screen.h"

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

    if (vm_screen_init() != OK) {
        fprintf(stderr, "Couldn't initialize video\n");
        exit(1);
    }
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

    vm_screen_finish();
}

/*
 * This is what will run when the program begins, if you were new to how
 * C works.
 */
int
main(int argc, char **argv)
{
    apple2 *mach;
    vm_screen *screen;
    int err;

    init(argc, argv);

    // When we exit, we want to wrap up a few loose ends. This syscall
    // will ensure that `finish()` runs whether we return from main
    // successfully or if we run `exit()` from elsewhere in the program.
    atexit(finish);

    mach = apple2_create();
    err = apple2_boot(mach);

    if (err != OK) {
        fprintf(stderr, "Bootup failed!\n");
        exit(1);
    }

    screen = vm_screen_create();
    if (screen == NULL) {
        fprintf(stderr, "Screen creation failed!\n");
        exit(1);
    }

    if (!vm_screen_add_window(screen, 640, 480)) {
        fprintf(stderr, "Window creation failed!\n");
        exit(1);
    }

    while (vm_screen_active(screen)) {
        vm_screen_set_color(screen, 255, 0, 0, 255);
        vm_screen_draw_rect(screen, 50, 50, 20, 20);
        vm_screen_refresh(screen);
    }

    // ha ha ha ha #nervous #laughter
    printf("Hello, world\n");
}
