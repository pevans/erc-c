/*
 * apple2.hires.c
 *
 * High resolution graphics in Apple are a significant change from its
 * low-resolution graphics. Where one byte can hold the color data for
 * two onscreen cells in lores graphics, in hires, each _bit_
 * corresponds to a pixel. The colors you can show depend on the pattern
 * of high and low bits within a given data byte. Certain rows have
 * black, purple, or blue available; alternating rows can be black,
 * green, or orange. 
 *
 * Some of this has to do with the space constraints available to the
 * Apple II: the hires graphics buffer is held between $2000 and $3FFF,
 * which is only 8k RAM. Some of this has to do with the peculiarities
 * of the NTSC format, because the Apple II was designed to work with
 * standard television screens.
 */

#include "apple2.hires.h"

void
apple2_hires_draw(apple2 *mach, size_t addr)
{
}
