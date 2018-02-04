#include <criterion/criterion.h>

#include "apple2.text.h"
#include "objstore.h"

/*
 * We're replicating the setup and teardown code from vm_bitfont.c so we
 * can test out the area draw
 */
static vm_bitfont *font;

static void
setup()
{
    vm_screen *screen;

    objstore_init();

    screen = vm_screen_create();
    font = vm_bitfont_create(screen, 
                             objstore_apple2_sysfont(), 
                             APPLE2_SYSFONT_SIZE,
                             7, 8, 0x7F);
}

static void
teardown()
{
    vm_bitfont_free(font);
}

TestSuite(apple2_text, .init = setup, .fini = teardown);

// Not a ton we can do for this--it's more or less a visual thing
/* Test(apple2_text, draw) */

Test(apple2_text, area)
{
    vm_area area;

    cr_assert_neq(apple2_text_area(&area, font, 0x0301), OK);
    cr_assert_neq(apple2_text_area(&area, font, 0x0461), OK);
    cr_assert_eq(apple2_text_area(&area, font, 0x0537), OK);

    // FIXME: need a proper test for area; we're missing one because the
    // yoff value I'm getting back is really out of whack, and I want to
    // do some more use-testing first
}
