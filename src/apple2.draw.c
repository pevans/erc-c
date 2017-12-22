/*
 * apple2.draw.c
 */

#include "apple2.h"

void
apple2_draw_pixel(apple2 *mach, vm_16bit addr)
{
}

// Drawing text is a two-step process; first you need to wipe out
// the block on the screen at that point. Then you need to draw the
// text in over that.

void
apple2_draw_text(apple2 *mach, vm_16bit addr)
{
    vm_8bit lsb, msb;
    vm_16bit page_base;
    SDL_Rect dest;
    char ch;

    // The text display buffers are located at "Page 1" and "Page 2",
    // which are at byte 1024-2047 (0x0400-0x07FF) and byte 2048-3071
    // (0x0800-0x0BFF) respectively. If the given address is not in
    // those (contiguous) ranges, then let's bail.
    if (addr < 0x0400 || addr > 0x0BFF) {
        return;
    }

    // In a given page for 40-column mode, you get 960 grid parts that
    // you may use. In 80-column mode, it's more like 1920 grid parts
    // (40x24 = 960, 80x24 = 1920). The way we look at this is the
    // address indicates the place on the grid where text should go. We
    // don't care how it got there. Let's figure out that position
    // on-screen.
    msb = (addr >> 8) & 0xff;   // row
    lsb = addr & 0xff;          // column

    // Regardless of which page we're rendering into, we can only use 40
    // cells on the grid (that is, 0-39 from whatever value the msb is).
    // It's possible to have an lsb greater than that, but if so, it's
    // not anything we can render to the screen.
    if (lsb > 39) {
        return;
    }

    if ((addr & 0xff80) % 128 != 0) {
        return;
    }

    // By default, we assume we're in text page 1. If the address ends
    // up being greater than 0x07FF, then we must be in page 2.
    page_base = 0x0400;
    if (addr > 0x07FF) {
        page_base = 0x0800;
    }

    // The absolute column position will be the font width times the
    // lsb.
    dest.x = lsb * mach->sysfont->width;

    // The absolute row position will be the font height times the msb
    // minus the page base (because the height is the same regardless of
    // what page we're in). So if we're msb $0400, then we're starting
    // on pixel row 0; but if we're msb $0480, then we are starting on
    // pixel row 8 (where the font height is 8); etc.
    dest.y = ((addr & 0xff80) - page_base) * mach->sysfont->height;

    // Our width and height must be that of the font.
    dest.w = mach->sysfont->width;
    dest.h = mach->sysfont->height;

    // And...lastly...what's in the address?
    ch = (char)vm_segment_get(mach->memory, addr);

    // Let's firstly blank out that space on screen.
    vm_bitfont_render(mach->sysfont, mach->screen, &dest, ' ');

    // Now show the goddamned thing
    vm_bitfont_render(mach->sysfont, mach->screen, &dest, ch);
}
