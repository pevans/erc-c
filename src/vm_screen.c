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
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        log_critical("Could not initialize video: %s", SDL_GetError());
        return ERR_GFXINIT;
    }

    return OK;
}

void
vm_screen_finish()
{
    SDL_Quit();
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

    screen->xcoords = 0;
    screen->ycoords = 0;

    screen->window = NULL;
    screen->render = NULL;
    screen->rect.x = 0;
    screen->rect.y = 0;
    screen->rect.w = 0;
    screen->rect.h = 0;

    return screen;
}

void
vm_screen_set_logical_coords(vm_screen *screen, int xcoords, int ycoords)
{
    screen->xcoords = xcoords;
    screen->ycoords = ycoords;

    // We allow you to set the x and y coordinates in absence of a
    // screen renderer, but it's kind of pointless without one.
    if (screen->render) {
        SDL_RenderSetLogicalSize(screen->render, 
                                 screen->xcoords, 
                                 screen->ycoords);
    }
}

int
vm_screen_add_window(vm_screen *screen, int width, int height)
{
#ifndef HEADLESS
    SDL_CreateWindowAndRenderer(
        width, height, 0, &screen->window, &screen->render);

    if (screen->window == NULL || screen->render == NULL) {
        log_critical("Could not create window: %s", SDL_GetError());
        return ERR_GFXINIT;
    }
#endif

    // We plan to draw onto a surface that is xcoords x ycoords in area,
    // regardless of the actual size of the window.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // We default to a logical coordinate system of exactly the given
    // width and height. For emulated systems like the Apple II, this
    // will not be correct, and the underlying machine system will rerun
    // the set_logical_coords function with different values.
    vm_screen_set_logical_coords(screen, width, height);

    vm_screen_set_color(screen, 0, 0, 0, 0);
    SDL_RenderClear(screen->render);

    return OK;
}

int
vm_screen_xcoords(vm_screen *screen)
{
    return screen->xcoords;
}

int
vm_screen_ycoords(vm_screen *screen)
{
    return screen->ycoords;
}

/*
 * Free the contents of a screen.
 */
void
vm_screen_free(vm_screen *screen)
{
    SDL_DestroyRenderer(screen->render);
    SDL_DestroyWindow(screen->window);
    free(screen);
}

bool
vm_screen_active(vm_screen *screen)
{
    static int counter = 5;

    if (counter--) {
        return true;
    }

    return false;
}

void
vm_screen_refresh(vm_screen *screen)
{
    SDL_RenderPresent(screen->render);
    SDL_Delay(2000);
}

/*
 * Set the color of a screen screen to a given RGBA value.
 */
void
vm_screen_set_color(vm_screen *screen,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue,
                    uint8_t alpha)
{
    if (screen->render) {
        SDL_SetRenderDrawColor(screen->render, red, green, blue, alpha);
    }
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
    // The renderer will take care of translating the positions and
    // sizes into whatever the window is really at.
    screen->rect.x = xpos;
    screen->rect.y = ypos;
    screen->rect.w = xsize;
    screen->rect.h = ysize;

    SDL_RenderFillRect(screen->render, &screen->rect);
}
