/*
 * vm_screen.c
 *
 * Generally speaking the tests here are very incomplete; this is partly
 * because a lot of the code in vm_screen depends on a third-party
 * graphics library, but I think we can do a better job of decoupling
 * some concepts (like screen areas) from the graphics library, such
 * that we can make the code that uses those testable.
 */

#include <criterion/criterion.h>

#include "vm_screen.h"

static vm_screen *screen;

static void
setup()
{
    screen = vm_screen_create();
}

static void
teardown()
{
    vm_screen_free(screen);
}

TestSuite(vm_screen, .init = setup, .fini = teardown);

/*
 * We're missing a good way of testing some functions here... I'm going
 * to list them for now. FIXME
 *
 * Test(vm_screen, init)
 * Test(vm_screen, finish)
 * Test(vm_screen, set_logical_coords)
 * Test(vm_screen, add_window)
 * Test(vm_screen, active)
 * Test(vm_screen, refresh)
 * Test(vm_screen, set_color)
 * Test(vm_screen, draw_rect)
 */

/* Test(vm_screen, free) */
Test(vm_screen, create) {
    cr_assert_neq(screen, NULL);

    cr_assert_eq(screen->window, NULL);
    cr_assert_eq(screen->render, NULL);
    cr_assert_eq(screen->xcoords, 0);
    cr_assert_eq(screen->ycoords, 0);
}

Test(vm_screen, xcoords)
{
    screen->xcoords = 123;
    cr_assert_eq(screen->xcoords, 123);
    screen->xcoords = 234;
    cr_assert_eq(screen->xcoords, 234);
}

Test(vm_screen, ycoords)
{
    screen->ycoords = 123;
    cr_assert_eq(screen->ycoords, 123);
    screen->ycoords = 234;
    cr_assert_eq(screen->ycoords, 234);
}

