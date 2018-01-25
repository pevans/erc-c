/*
 * apple2.text.c
 *
 * This code sits as a kind of wrapper over the bitmap fonts that the
 * Apple II uses. Neither the system nor inverted fonts are exactly
 * composed in the type of character set that Apple expects; they
 * implement the glpyhs we need in order to make such. You can have
 * inversed characters in the primary character set, for one; in the
 * alternate character set, you can have MouseText glyphs, yet they are
 * implemented as part of both fonts.
 */

#include <ctype.h>

#include "apple2.text.h"

/*
 * Draw a text character at the given address.
 */
void
apple2_text_draw(apple2 *mach, size_t addr)
{
    int err, bit7, bit6;
    vm_8bit ch;
    vm_area dest;
    vm_bitfont *font;

    // If we're updating a page 2 address and we're not in some kind of
    // double resolution mode, then we shouldn't actually render the
    // thing.
    if (addr > 0x07FF && !apple2_is_double_video(mach)) {
        return;
    }

    // Default
    font = mach->sysfont;

    err = apple2_text_area(&dest, font, addr);
    if (err != OK) {
        return;
    }

    // What are we working with?
    ch = mos6502_get(mach->cpu, addr);

    // Bit 7 and bit 6 mean different things in ALTCHAR
    // and...non-ALTCHAR. 
    bit7 = ch & 0x80;
    bit6 = ch & 0x40;

    // The ASCII code we will use is only that which is composed of the
    // first 6 bits.
    ch = ch & 0x7f;

    // We treat special characters as spaces for display purposes.
    if (ch < 0x20) {
        vm_bitfont_render(font, mach->screen, &dest, ' ');
        return;
    }

    // If bit 7 is high, then we want to show inverse video. This is
    // mostly true regardless of character set.
    if (bit7) {
        font = mach->invfont;
    }

    if (mach->display_mode & DISPLAY_ALTCHAR) {
        // The character here should be MouseText, so we'll need to use
        // the _non_ inverted font for that.
        if (ch >= 0x40 && ch <= 0x5F) {
            font = mach->sysfont;
        }

        if (bit6) {
            ch = tolower(ch);
        }
    } else {
        // FIXME: we should turn on flashing text here, but that's not
        // yet implemented
        if (bit6) {
            // do something with flashing text
        }
    }

    // Blank out the space on the screen, then show the character
    vm_bitfont_render(font, mach->screen, &dest, ' ');
    vm_bitfont_render(font, mach->screen, &dest, ch);
}

/*
 * This function will fill in the width, height, and x and y offsets for
 * a character at the given address using the given font.
 */
int
apple2_text_area(vm_area *area, vm_bitfont *font, size_t addr)
{
    vm_8bit lsb;
    int page_base;

    // The text display buffers are located at "Page 1" and "Page 2",
    // which are at byte 1024-2047 (0x0400-0x07FF) and byte 2048-3071
    // (0x0800-0x0BFF) respectively. If the given address is not in
    // those (contiguous) ranges, then let's bail.
    if (addr < 0x0400 || addr > 0x0BFF) {
        return 0;
    }

    // In a given page for 40-column mode, you get 960 grid parts that
    // you may use. In 80-column mode, it's more like 1920 grid parts
    // (40x24 = 960, 80x24 = 1920). The way we look at this is the
    // address indicates the place on the grid where text should go. We
    // don't care how it got there. Let's figure out that position
    // on-screen.
    lsb = addr & 0xff;          // column

    // Regardless of which page we're rendering into, we can only use 40
    // cells on the grid (that is, 0-39 from whatever value the msb is).
    // It's possible to have an lsb greater than that, but if so, it's
    // not anything we can render to the screen.
    if (lsb > 0x39) {
        return 0;
    }

    if ((addr & 0xff80) % 128 != 0) {
        return 0;
    }

    // By default, we assume we're in text page 1. If the address ends
    // up being greater than 0x07FF, then we must be in page 2.
    page_base = 0x0400;
    if (addr > 0x07FF) {
        page_base = 0x0800;
    }

    // The absolute column position will be the font width times the
    // lsb.
    area->xoff = lsb * font->width;

    // The absolute row position will be the font height times the msb
    // minus the page base (because the height is the same regardless of
    // what page we're in). So if we're msb $0400, then we're starting
    // on pixel row 0; but if we're msb $0480, then we are starting on
    // pixel row 8 (where the font height is 8); etc.
    area->yoff = ((addr & 0xff80) - page_base) * font->height;

    // Our width and height must be that of the font.
    area->width = font->width;
    area->height = font->height;

    return OK;
}
