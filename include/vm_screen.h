#ifndef _VM_SCREEN_H_
#define _VM_SCREEN_H_

typedef struct {
    int color_red;
    int color_green;
    int color_blue;
    int color_alpha;
} vm_screen_context;

extern void vm_screen_draw_rect(vm_screen_context *, int, int, int, int);
extern void vm_screen_free_context(vm_screen_context *);
extern vm_screen_context *vm_screen_new_context();
extern void vm_screen_set_color(vm_screen_context *, int, int, int, int);

#define vm_screen_draw_pixel(context, xpos, ypos) \
    vm_screen_draw_rect(context, xpos, ypos, 1, 1)

#endif
