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

#include "apple2.text.h"

void
apple2_text_draw(apple2 *mach, size_t addr)
{
    int err;
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

    // We treat special characters as spaces for display purposes.
    if (ch < 0x20) {
        vm_bitfont_render(font, mach->screen, &dest, ' ');
        return;
    }

    // Blank out the space on the screen, then show the character
    vm_bitfont_render(font, mach->screen, &dest, ' ');
    vm_bitfont_render(font, mach->screen, &dest, ch);
}

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
