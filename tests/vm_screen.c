#include <criterion/criterion.h>

#include "vm_screen.h"

Test(vm_screen, new_context) {
    vm_screen_context *context;

    context = vm_screen_new_context();
    cr_assert_neq(context, NULL);

    cr_assert_eq(context->color_red, 0);
    cr_assert_eq(context->color_blue, 0);
    cr_assert_eq(context->color_green, 0);
    cr_assert_eq(context->color_alpha, 0);

    vm_screen_free_context(context);
}

Test(vm_screen, set_color) {
    vm_screen_context *context;
    int red = 0xDE;
    int green = 0xAD;
    int blue = 0xBE;
    int alpha = 0xEF;

    context = vm_screen_new_context();
    vm_screen_set_color(context, red, green, blue, alpha);

    cr_assert_eq(context->color_red, red);
    cr_assert_eq(context->color_green, green);
    cr_assert_eq(context->color_blue, blue);
    cr_assert_eq(context->color_alpha, alpha);

    vm_screen_free_context(context);
}

Test(vm_screen, draw_rect) {
    // Nothing to do here...
}
