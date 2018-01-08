/*
 * vm_area.c
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
