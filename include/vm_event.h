#ifndef _VM_EVENT_H_
#define _VM_EVENT_H_

#include "vm_screen.h"

typedef struct {
    SDL_Event event;
    vm_screen *screen;
} vm_event;

extern void vm_event_keyboard(vm_event *);
extern void vm_event_poll(vm_screen *);

#endif
