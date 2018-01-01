/*
 * apple2.draw.c
 */

#include "apple2.h"

static int lores_colors[][3] = {
    { 0x00, 0x00, 0x00 },   // black
    { 0xff, 0x28, 0x97 },   // magenta
    { 0x60, 0x4d, 0xbc },   // dark blue
    { 0xff, 0x44, 0xfd },   // purple
    { 0x00, 0xa3, 0x60 },   // dark green
    { 0x9c, 0x9c, 0x9c },   // gray
    { 0x14, 0xcf, 0xfd },   // medium blue
    { 0xd0, 0xc3, 0xff },   // light blue
    { 0x60, 0x72, 0x03 },   // brown
    { 0xff, 0x6a, 0x3c },   // orange
    { 0x9c, 0x9c, 0x9c },   // gray
    { 0xff, 0xa0, 0xd0 },   // pink
    { 0x14, 0xf5, 0x3c },   // light green
    { 0xd0, 0xdd, 0x81 },   // yellow
    { 0x72, 0xff, 0xd0 },   // aquamarine
    { 0xff, 0xff, 0xff },   // white
};

void
apple2_draw_pixel(apple2 *mach, vm_16bit addr)
{
}

/*
 * In single lo-res mode, pixels are 40x40 on the screen. But since our
 * single mode has 280 pixels that _we_ show, this means that a pixel in
 * single lo-res is really 7 columns wide. (Confusing yet?) Each "row" in
 * text page 1 is just a row of visible dots, and 4 rows are equivalent
 * to one "pixel" in lo-res mode.
 *
 * Without text, you get 40x48 pixels.
 */
void
apple2_draw_pixel_lores(apple2 *mach, vm_16bit addr)
{
    vm_8bit color = vm_segment_get(mach->memory, addr);
    vm_8bit top, bottom;
    vm_area loc;
    int *colors;

    // The top color is the low order nibble, so we can blank out the
    // high order nibble by AND-ing a mask of 0x0f (which is b00001111,
    // 15 decimal). The bottom color, ergo, is the high order nibble;
    // for that, we simply shift the color 4 positions to the right.
    top = color & 0x0F;
    bottom = color >> 4;

    // The next thing we need to consider is where we draw the pixel
    loc.xoff = (addr & 0xff) * mach->sysfont->width;
    loc.yoff = (addr >> 8) * mach->sysfont->height;
    loc.width = mach->sysfont->width;
    loc.height = mach->sysfont->height / 2;

    colors = lores_colors[top];
    vm_screen_set_color(mach->screen, colors[0], colors[1], colors[2], 255);
    vm_screen_draw_rect(mach->screen, &loc); 

    // The bottom pixel we need to draw now must be offset by the height
    // of the pixel we just drew. (Remember, positive equals down in the
    // y-axis.)
    loc.yoff += loc.height;

    colors = lores_colors[bottom];
    vm_screen_set_color(mach->screen, colors[0], colors[1], colors[2], 255);
    vm_screen_draw_rect(mach->screen, &loc);
}

void
apple2_draw_text(apple2 *mach, vm_16bit addr)
{
    vm_8bit lsb;
    vm_16bit page_base;
    vm_area dest;
    char ch;

    // The text display buffers are located at "Page 1" and "Page 2",
    // which are at byte 1024-2047 (0x0400-0x07FF) and byte 2048-3071
    // (0x0800-0x0BFF) respectively. If the given address is not in
    // those (contiguous) ranges, then let's bail.
    if (addr < 0x0400 || addr > 0x0BFF) {
        return;
    }

    // If we're updating a page 2 address and we're not in some kind of
    // double resolution mode, then we shouldn't actually render the
    // thing.
    if (addr > 0x07FF && !apple2_is_double_video(mach)) {
        return;
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
    dest.xoff = lsb * mach->sysfont->width;

    // The absolute row position will be the font height times the msb
    // minus the page base (because the height is the same regardless of
    // what page we're in). So if we're msb $0400, then we're starting
    // on pixel row 0; but if we're msb $0480, then we are starting on
    // pixel row 8 (where the font height is 8); etc.
    dest.yoff = ((addr & 0xff80) - page_base) * mach->sysfont->height;

    // Our width and height must be that of the font.
    dest.width = mach->sysfont->width;
    dest.height = mach->sysfont->height;

    // And...lastly...what's in the address?
    ch = (char)vm_segment_get(mach->memory, addr);

    // Let's firstly blank out that space on screen.
    vm_bitfont_render(mach->sysfont, mach->screen, &dest, ' ');

    // Now show the goddamned thing
    vm_bitfont_render(mach->sysfont, mach->screen, &dest, ch);
}
