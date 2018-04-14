/*
 * apple2.lores.c
 *
 * The Apple II has several modes of display, and one of those modes is
 * low-resolution graphics. Graphics are shown on a 40x48 grid, where
 * each cell in the grid consumes 4 rows or columns of pixels.
 *
 * It's possible in low-resolution graphics (and high-resolution, for
 * that matter) to show text with graphics--that is to say, exactly the
 * kind of text that would be seen in 40-column text mode. If text is
 * shown (this is indicated through a "mixed" mode flag), then it will
 * be rendered at the bottom of the screen, and the rows of pixels that
 * would have been rendered in full graphical mode will be skipped.
 */

#include "apple2/apple2.h"
#include "apple2/lores.h"
#include "apple2/text.h"

/*
 * These are the color codes for the lo-res colors that are available.
 * Each byte in lo-res represents two visual cells, each 4 bytes high,
 * which are indicated by considering the low and high 4 bits separately
 * and matching them against the colors in the table below.
 *
 * These color values are the "NTSC Corrected" colors as defined in the
 * Apple II Forever Anthology, v0.3, Hires reference (pulled from
 * archive.org).
 */
static vm_color lores_colors[] = {
    { 0x00, 0x00, 0x00, 0x00 },   // black
    { 0xff, 0x28, 0x97, 0x00 },   // magenta
    { 0x60, 0x4d, 0xbc, 0x00 },   // dark blue
    { 0xff, 0x44, 0xfd, 0x00 },   // purple
    { 0x00, 0xa3, 0x60, 0x00 },   // dark green
    { 0x9c, 0x9c, 0x9c, 0x00 },   // gray
    { 0x14, 0xcf, 0xfd, 0x00 },   // medium blue
    { 0xd0, 0xc3, 0xff, 0x00 },   // light blue
    { 0x60, 0x72, 0x03, 0x00 },   // brown
    { 0xff, 0x6a, 0x3c, 0x00 },   // orange
    { 0x9c, 0x9c, 0x9c, 0x00 },   // gray
    { 0xff, 0xa0, 0xd0, 0x00 },   // pink
    { 0x14, 0xf5, 0x3c, 0x00 },   // light green
    { 0xd0, 0xdd, 0x81, 0x00 },   // yellow
    { 0x72, 0xff, 0xd0, 0x00 },   // aquamarine
    { 0xff, 0xff, 0xff, 0x00 },   // white
};


/*
 * Draw two cells on the low-resolution grid. Each byte address holds
 * two cells-worth of data. We may delegate the render to the text
 * render code if we're in mixed mode.
 */
void
apple2_lores_draw(apple2 *mach, size_t addr)
{
    int row = apple2_text_row(addr),
        col = apple2_text_col(addr);

    if (row == -1 || col == -1) {
        return;
    }

    // In low-resolution mode, the row number is analagous to the text
    // row number, like so:
    row *= 2;

    // In mixed display mode, we have to render text in the lower rows
    // of the screen
    if ((mach->display_mode & DISPLAY_MIXED) && row >= 40) {
        apple2_text_draw(mach, addr);
        return;
    }

    vm_8bit byte = mos6502_get(mach->cpu, addr);
    vm_8bit topcell = byte & 0xF,
            botcell = byte >> 4;

    vm_area dest;

    // Cells are more like rectangles than squares
    dest.width = 7;
    dest.height = 4;
    dest.xoff = 7 * col;
    dest.yoff = 4 * row;

    // Draw the top cell
    vm_screen_set_color(mach->screen, apple2_lores_color(topcell));
    vm_screen_draw_rect(mach->screen, &dest);

    // And draw the bottom cell
    dest.yoff += 4;
    vm_screen_set_color(mach->screen, apple2_lores_color(botcell));
    vm_screen_draw_rect(mach->screen, &dest);
}

/*
 * Return the color that corresponds to the value in the given byte.
 * Even though we receive a byte, lores color data is stored two cells
 * to a byte, so all we ever get here will be contained within the first
 * four bits; therefore we mask the byte as we index the lores colors
 * table, to ensure a bad value doesn't end up returning a value out of
 * bounds.
 */
vm_color
apple2_lores_color(vm_8bit byte)
{
    return lores_colors[byte & 0xf];
}
