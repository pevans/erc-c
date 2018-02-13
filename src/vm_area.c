/*
 * vm_area.c
 *
 * The area code defines a library-independent "area" struct, because we
 * don't want to write virtual machine code which makes literal use of
 * SDL_Rects. Any time you need to pass a screen area (which would be a
 * rectangle of a certain width and height, offset from the top-left of
 * the screen by a given x/y coordinate), you would use a vm_area
 * struct to do so.
 */

#include "vm_area.h"

/*
 * Assign the values of an area, which are an x and y offset plus a
 * width and height.
 */
inline void
vm_area_set(vm_area *area, int xoff, int yoff, int width, int height)
{
    area->xoff = xoff;
    area->yoff = yoff;
    area->width = width;
    area->height = height;
}
