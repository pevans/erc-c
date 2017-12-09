/*
 * vm_screen.c
 *
 * Functions here support drawing to the virtual machine's "screen";
 * exactly how that is done is an abstraction to the rest of the
 * program, which only knows to call the functions here.
 */

#include <stdlib.h>

#include "log.h"
#include "vm_screen.h"

/*
 * Return a new screen context. We also set the color to black.
 */
vm_screen_context *
vm_screen_new_context()
{
    vm_screen_context *context;

    context = (vm_screen_context *)malloc(sizeof(vm_screen_context));
    if (context == NULL) {
        log_critical("Failed to allocate vm_screen_context");
        exit(1);
    }

    vm_screen_set_color(context, 0, 0, 0, 0);
    return context;
}

/*
 * Free the contents of a screen context.
 */
void
vm_screen_free_context(vm_screen_context *context)
{
    free(context);
}

/*
 * Set the color of a screen context to a given RGBA value.
 */
void
vm_screen_set_color(vm_screen_context *context,
                    int red,
                    int green,
                    int blue,
                    int alpha)
{
    context->color_red = red;
    context->color_green = green;
    context->color_blue = blue;
    context->color_alpha = alpha;
}

/*
 * Draw a rectangle on the screen at a given x/y position, with a given
 * set of x/y dimensions, with a given screen context.
 */
void
vm_screen_draw_rect(vm_screen_context *context,
                    int xpos,
                    int ypos,
                    int xsize,
                    int ysize)
{
    // FIXME: NOOP
}
