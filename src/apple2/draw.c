/*
 * apple2.draw.c
 *
 * Draw pretty pixels on the screen in the way that Apple II would work.
 * NB: this code is pretty rough, and I expect some changes as we get
 * further into development.
 */

#include "apple2/apple2.h"
#include "apple2/hires.h"
#include "apple2/lores.h"
#include "apple2/text.h"

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
#if 0
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
#endif
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
 * Draw low-resolution graphics on the screen
 */
void
apple2_draw_lores(apple2 *mach)
{
    size_t addr;

    vm_screen_prepare(mach->screen);
    
    for (addr = 0x400; addr < 0x800; addr++) {
        apple2_lores_draw(mach, addr);
    }
}

/*
 * Draw high-resolution graphics on the screen
 */
void
apple2_draw_hires(apple2 *mach)
{
    vm_screen_prepare(mach->screen);

    for (int row = 0; row < 192; row++) {
        apple2_hires_draw(mach, row);
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
        return;
    } else if (mach->memory_mode & MEMORY_HIRES) {
        apple2_draw_hires(mach);
        return;
    }

    // The fallback mode is to draw lores graphics
    apple2_draw_lores(mach);
}
