#ifndef _VM_SCREEN_H_
#define _VM_SCREEN_H_

#include <stdbool.h>
#include <SDL.h>

#include "vm_area.h"

typedef struct {
    /*
     * This is the window in SDL that we're displaying. It's fine for a
     * screen to be headless; that is, not to have a window. Screen
     * functions which deal with SDL will simply not run that code if
     * headless.
     */
    SDL_Window *window;

    /*
     * In SDL, the renderer is comparable to the old SDL_Surface type
     * (which is still there!). A renderer is a little more stateful; it
     * contains its own color information to be used when rendering
     * shapes, for example.
     */
    SDL_Renderer *render;

    /*
     * These are the x and y coordinates of the window we're creating.
     * FIXME: this should probably be renamed to width and height...
     */
    int xcoords;
    int ycoords;
} vm_screen;

extern bool vm_screen_active(vm_screen *);
extern int vm_screen_add_window(vm_screen *, int, int);
extern int vm_screen_init();
extern int vm_screen_xcoords(vm_screen *);
extern int vm_screen_ycoords(vm_screen *);
extern vm_screen *vm_screen_create();
extern void vm_screen_draw_rect(vm_screen *, vm_area *);
extern void vm_screen_finish();
extern void vm_screen_free(vm_screen *);
extern void vm_screen_refresh(vm_screen *);
extern void vm_screen_set_color(vm_screen *, uint8_t, uint8_t, uint8_t, uint8_t);
extern void vm_screen_set_logical_coords(vm_screen *, int, int);

#endif
