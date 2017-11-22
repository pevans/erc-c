#include <stdlib.h>

#include "log.h"
#include "vm_screen.h"

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

void
vm_screen_draw_rect(vm_screen_context *context,
                    int xpos,
                    int ypos,
                    int xsize,
                    int ysize)
{
    // FIXME: NOOP
}
