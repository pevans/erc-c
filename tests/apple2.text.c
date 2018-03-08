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
    cr_assert_neq(apple2_text_area(&area, font, 0x04FB), OK);
    cr_assert_eq(apple2_text_area(&area, font, 0x0537), OK);

    // FIXME: need a proper test for area; we're missing one because the
    // yoff value I'm getting back is really out of whack, and I want to
    // do some more use-testing first
}

Test(apple2_text, primary)
{
    static char upper[] = {
        '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
        ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    };

    static char lower[] = {
        '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ', 
    };

    for (int i = 0; i < sizeof(upper); i++) {
        cr_assert_eq(apple2_text_primary(i), upper[i]);
    }

    for (int i = 0; i < sizeof(lower); i++) {
        cr_assert_eq(apple2_text_primary(i + 0xe0), lower[i]);
    }
}

Test(apple2_text, alternate)
{
    static char upper[] = {
        '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
        ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    };
    
    static char lower[] = {
        '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ', 
    };

    for (int i = 0; i < sizeof(upper); i++) {
        cr_assert_eq(apple2_text_alternate(i + 0x80), upper[i]);
    }

    for (int i = 0; i < sizeof(lower); i++) {
        cr_assert_eq(apple2_text_alternate(i + 0x60), lower[i]);
    }
}
