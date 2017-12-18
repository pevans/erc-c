#include <criterion/criterion.h>

#include "vm_screen.h"

Test(vm_screen, create) {
    vm_screen *screen;
    int x = 320;
    int y = 240;
    int scale = 2;

    screen = vm_screen_create(x, y, scale);
    cr_assert_neq(screen, NULL);

    cr_assert_eq(screen->xcoords, x);
    cr_assert_eq(screen->ycoords, y);
    cr_assert_eq(screen->scale, scale);

    cr_assert_eq(screen->window, NULL);
    cr_assert_eq(screen->render, NULL);
    cr_assert_eq(screen->rect.x, 0);
    cr_assert_eq(screen->rect.y, 0);
    cr_assert_eq(screen->rect.w, 0);
    cr_assert_eq(screen->rect.h, 0);

    vm_screen_free(screen);
}
