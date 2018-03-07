/*
 * apple2.draw.c
 *
 * Draw pretty pixels on the screen in the way that Apple II would work.
 * NB: this code is pretty rough, and I expect some changes as we get
 * further into development.
 */

#include "apple2.h"
#include "apple2.text.h"

/*
 * These are the color codes for the lo-res colors that are available.
 * Each pixel in lo-res indicates one color. These are colors I found
 * somewhere online -- I'm not sure if they are exact matches, and are
 * subject to change.
 */
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

/*
 * Draw a pixel on screen at the given address.
 * FIXME: we do not draw anything D:
 */
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
    vm_8bit color = vm_segment_get(mach->main, addr);
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

/*
 * Draw the 40-column text necessary to render everything on the screen
 * with the machine in its current state.
 */
void
apple2_draw_40col(apple2 *mach)
{
    size_t addr;

    vm_screen_prepare(mach->screen);

    for (addr = 0x400; addr < 0x800; addr++) {
        apple2_text_draw(mach, addr);
    }
}

/*
 * Find the right draw method for the machine, based on its display
 * mode, and use that to refresh the screen.
 */
void
apple2_draw(apple2 *mach)
{
    if (mach->display_mode & DISPLAY_TEXT) {
        apple2_draw_40col(mach);
    }
}

/*
 * Tell the screen that it has a change ready to be displayed. This
 * function will not immediately redraw the screen, and when the screen
 * is redrawn is up to our framerate cycle.
 */
void
apple2_notify_refresh(apple2 *mach)
{
    if (mach && mach->screen) {
        mach->screen->dirty = true;
    }
}
