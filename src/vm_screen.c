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
vm_screen_create(int xcoords, int ycoords, int scale)
{
    vm_screen *screen;

    screen = (vm_screen *)malloc(sizeof(vm_screen));
    if (screen == NULL) {
        log_critical("Failed to allocate vm_screen");
        exit(1);
    }

    screen->xcoords = xcoords;
    screen->ycoords = ycoords;
    screen->scale = scale;

    screen->window = NULL;
    screen->render = NULL;
    screen->rect.x = 0;
    screen->rect.y = 0;
    screen->rect.w = 0;
    screen->rect.h = 0;

    return screen;
}

int
vm_screen_add_window(vm_screen *screen)
{
    SDL_CreateWindowAndRenderer(screen->xcoords * screen->scale,
                                screen->ycoords * screen->scale,
                                0, &screen->window, &screen->render);

    if (screen->window == NULL || screen->render == NULL) {
        log_critical("Could not create window: %s", SDL_GetError());
        return ERR_GFXINIT;
    }

    // We plan to draw onto a surface that is xcoords x ycoords in area,
    // regardless of the actual size of the window.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(screen->render, 
                             screen->xcoords, 
                             screen->ycoords);

    vm_screen_set_color(screen, 0, 0, 0, 0);
    SDL_RenderClear(screen->render);

    return OK;
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
    SDL_SetRenderDrawColor(screen->render, red, green, blue, alpha);
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
