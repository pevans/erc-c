#ifndef _VM_EVENT_H_
#define _VM_EVENT_H_

#include "vm_screen.h"

typedef struct {
    SDL_Event event;
    vm_screen *screen;
} vm_event;

typedef void (*vm_event_fn) (void *);

#define EVENT_DO(x) \
    void x(void *_mach)

extern int vm_event_do(int);
extern void vm_event_keyboard(vm_event *);
extern void vm_event_keyboard_normal(vm_event *, char);
extern void vm_event_keyboard_special(vm_event *, char);
extern void vm_event_poll(vm_screen *);

#endif
