#include <criterion/criterion.h>

#include "vm_screen.h"

Test(vm_screen, create) {
    vm_screen *screen;

    screen = vm_screen_create();
    cr_assert_neq(screen, NULL);

    cr_assert_eq(screen->color_red, 0);
    cr_assert_eq(screen->color_blue, 0);
    cr_assert_eq(screen->color_green, 0);
    cr_assert_eq(screen->color_alpha, 0);

    vm_screen_free(screen);
}

Test(vm_screen, set_color) {
    vm_screen *screen;
    int red = 0xDE;
    int green = 0xAD;
    int blue = 0xBE;
    int alpha = 0xEF;

    screen = vm_screen_create();
    vm_screen_set_color(screen, red, green, blue, alpha);

    cr_assert_eq(screen->color_red, red);
    cr_assert_eq(screen->color_green, green);
    cr_assert_eq(screen->color_blue, blue);
    cr_assert_eq(screen->color_alpha, alpha);

    vm_screen_free(screen);
}

Test(vm_screen, draw_rect) {
    // Nothing to do here...
}
