#ifndef _VM_SCREEN_H_
#define _VM_SCREEN_H_

#include <stdbool.h>
#include <SDL.h>

#define VM_SCREEN_DEFWIDTH 800
#define VM_SCREEN_DEFHEIGHT 600

/*
 * If you just want to plot a single pixel, you can use this macro to
 * abstract away the need to indicate the x/y dimensions (as those must
 * necessarily be 1x1).
 */
#define vm_screen_draw_pixel(screen, xpos, ypos) \
    vm_screen_draw_rect(screen, xpos, ypos, 1, 1)

typedef struct {
    int xoff;
    int yoff;

    int width;
    int height;
} vm_area;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *render;
    vm_area area;

    int xcoords;
    int ycoords;
} vm_screen;

#define SET_SDL_RECT(name, a) \
    (name).x = (a).xoff; \
    (name).y = (a).yoff; \
    (name).w = (a).width; \
    (name).h = (a).height

#define MAKE_SDL_RECT(name, a) \
    SDL_Rect name; \
    SET_SDL_RECT(name, a)

#define vm_area_set(a, x, y, w, h) \
    (a)->xoff = x; \
    (a)->yoff = y; \
    (a)->width = w; \
    (a)->height = h

extern bool vm_screen_active(vm_screen *);
extern int vm_screen_add_window(vm_screen *, int, int);
extern int vm_screen_init();
extern int vm_screen_xcoords(vm_screen *);
extern int vm_screen_ycoords(vm_screen *);
extern vm_screen *vm_screen_create();
extern void vm_screen_draw_rect(vm_screen *, int, int, int, int);
extern void vm_screen_finish();
extern void vm_screen_free(vm_screen *);
extern void vm_screen_refresh(vm_screen *);
extern void vm_screen_set_color(vm_screen *, uint8_t, uint8_t, uint8_t, uint8_t);
extern void vm_screen_set_logical_coords(vm_screen *, int, int);

#endif
