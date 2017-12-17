#ifndef _VM_SCREEN_H_
#define _VM_SCREEN_H_

#include <GLFW/glfw3.h>
#include <stdbool.h>

/*
 * If you just want to plot a single pixel, you can use this macro to
 * abstract away the need to indicate the x/y dimensions (as those must
 * necessarily be 1x1).
 */
#define vm_screen_draw_pixel(context, xpos, ypos) \
    vm_screen_draw_rect(context, xpos, ypos, 1, 1)

typedef struct {
    GLFWwindow *window;

    /*
     * These form the components of an RGBA composite color. 
     */
    int color_red;
    int color_green;
    int color_blue;
    int color_alpha;
} vm_screen_context;

extern int vm_screen_add_window(vm_screen_context *);
extern int vm_screen_init();
extern void vm_screen_finish();
extern void vm_screen_draw_rect(vm_screen_context *, int, int, int, int);
extern void vm_screen_free_context(vm_screen_context *);
extern vm_screen_context *vm_screen_new_context();
extern void vm_screen_set_color(vm_screen_context *, int, int, int, int);

#endif
