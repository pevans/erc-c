/*
 * apple2.draw.c
 */

void
apple2_draw_pixel(apple2 *mach, vm_16bit addr)
{
}

void
apple2_draw_text(apple2 *mach, vm_16bit addr)
{
    // Drawing text is a two-step process; first you need to wipe out
    // the block on the screen at that point. Then you need to draw the
    // text in over that.
}
