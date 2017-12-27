/*
 * vm_bitfont.c
 */

#include <criterion/criterion.h>

#include "vm_bitfont.h"

static vm_bitfont *font;

static void
setup()
{
    vm_screen *screen;

    screen = vm_screen_create();
    font = vm_bitfont_create(screen, "apple2-system", 7, 8, 0x7F);
}

static void
teardown()
{
    vm_bitfont_free(font);
}

TestSuite(vm_bitfont, .init = setup, .fini = teardown);

Test(vm_bitfont, create)
{
    cr_assert_neq(font, NULL);

    cr_assert_eq(font->width, 7);
    cr_assert_eq(font->height, 8);
    cr_assert_eq(font->cmask, 0x7F);

    // This should be NULL because we have no screen
    cr_assert_eq(font->texture, NULL);
}

Test(vm_bitfont, offset)
{
    char ch = 'p';
    vm_area area;

    vm_bitfont_offset(font, ch, &area);
    
    cr_assert_eq(area.xoff, (ch & 0x0f) * font->width);
    cr_assert_eq(area.yoff, (ch >> 4) * font->height);
}

/*
 * A note: I omitted a test for vm_bitfont_render(), as this function is
 * a) reliant on vm_bitfont_offset for the right glyph to render, and b)
 * is otherwise reliant on our graphics library to properly render the
 * character. As we run our tests in a headless mode, we are unable to
 * automate that specific kind of test.
 */
Test(vm_bitfont, render)
{
}
