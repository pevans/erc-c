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
    SDL_Window *window;
    SDL_Renderer *render;
    SDL_Rect rect;

    int xcoords;
    int ycoords;

    int scale;
} vm_screen;

extern int vm_screen_add_window(vm_screen *);
extern int vm_screen_init();
extern void vm_screen_finish();
extern void vm_screen_refresh(vm_screen *);
extern bool vm_screen_active(vm_screen *);
extern void vm_screen_draw_rect(vm_screen *, int, int, int, int);
extern void vm_screen_free(vm_screen *);
extern vm_screen *vm_screen_create();
extern void vm_screen_set_color(vm_screen *, uint8_t, uint8_t, uint8_t, uint8_t);

#endif
