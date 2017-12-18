/*
 * vm_screen.c
 *
 * Functions here support drawing to the virtual machine's "screen";
 * exactly how that is done is an abstraction to the rest of the
 * program, which only knows to call the functions here.
 */

#include <stdbool.h>
#include <stdlib.h>

#include "log.h"
#include "vm_screen.h"

int
vm_screen_init()
{
    return OK;
}

void
vm_screen_finish()
{
}

/*
 * Return a new screen. We also set the color to black.
 */
vm_screen *
vm_screen_create()
{
    vm_screen *screen;

    screen = (vm_screen *)malloc(sizeof(vm_screen));
    if (screen == NULL) {
        log_critical("Failed to allocate vm_screen");
        exit(1);
    }

    vm_screen_set_color(screen, 0, 0, 0, 0);
    return screen;
}

int
vm_screen_add_window(vm_screen *screen)
{
    return OK;
}

/*
 * Free the contents of a screen.
 */
void
vm_screen_free(vm_screen *screen)
{
    free(screen);
}

bool
vm_screen_active(vm_screen *screen)
{
    return false;
}

void
vm_screen_refresh(vm_screen *screen)
{
}

/*
 * Set the color of a screen screen to a given RGBA value.
 */
void
vm_screen_set_color(vm_screen *screen,
                    int red,
                    int green,
                    int blue,
                    int alpha)
{
    screen->color_red = red;
    screen->color_green = green;
    screen->color_blue = blue;
    screen->color_alpha = alpha;
}

/*
 * Draw a rectangle on the screen at a given x/y position, with a given
 * set of x/y dimensions, with a given screen.
 */
void
vm_screen_draw_rect(vm_screen *screen,
                    int xpos,
                    int ypos,
                    int xsize,
                    int ysize)
{
    // FIXME: NOOP
}
