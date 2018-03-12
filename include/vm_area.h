#ifndef _VM_AREA_H_
#define _VM_AREA_H_

#include <stdlib.h>

typedef struct {
    /*
     * These are the x and y coordinate offsets in the logical dimension
     * established in a vm_screen. An offset of (0, 0) would be in the
     * top-left; (5, 5) would be 5 pixels down, and 5 pixels to the
     * right, of that top-left corner.
     */
    uint32_t xoff;
    uint32_t yoff;

    /*
     * These are the width and height of the area we're defining. A
     * single pixel in the logical area would have a width and height of
     * (1, 1); use larger numbers to indicate a larger square (if the
     * two are equal) or rectangle (if unequal).
     */
    uint32_t width;
    uint32_t height;
} vm_area;

/*
 * Set the contents of an SDL_Rect to the equivalent fields contained in
 * a vm_area.
 */
#define SET_SDL_RECT(name, a) \
    (name).x = (a).xoff; \
    (name).y = (a).yoff; \
    (name).w = (a).width; \
    (name).h = (a).height

/*
 * Much like SET_SDL_RECT(), except this will (as a side-effect!)
 * declare an SDL_Rect variable (`name`) and pass that into the SET
 * macro.
 */
#define MAKE_SDL_RECT(name, a) \
    SDL_Rect name; \
    SET_SDL_RECT(name, a)

extern void vm_area_set(vm_area *, uint32_t, uint32_t, uint32_t, uint32_t);

#endif
