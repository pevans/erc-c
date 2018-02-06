/*
 * vm_event.c
 */

#include "vm_event.h"

/*
 * Look through all of the events that are queued up and, whatever we
 * need to do for them, do that.
 */
void
vm_event_poll(vm_screen *scr)
{
    vm_event ev;

    ev.screen = scr;
    while (SDL_PollEvent(&ev.event)) {
        if (ev.event.type == SDL_KEYDOWN || ev.event.type == SDL_KEYUP) {
            vm_event_keyboard(&ev);
        }
    }
}

/*
 * Handle any keyboard events from the event queue. Those would be
 * things like pressing a key, releasing a key... boring stuff, really.
 */
void
vm_event_keyboard(vm_event *ev)
{
    char ch;

    // The sym field is of type SDL_Keycode; this type, however,
    // maps roughly to Unicode, which of course maps roughly to
    // ASCII in the low range.
    ch = (char)ev->event.key.keysym.sym;

    // If we had shift pressed, we need to uppercase the
    // character.
    if (ev->event.key.keysym.mod & KMOD_LSHIFT ||
        ev->event.key.keysym.mod & KMOD_RSHIFT
       ) {
        ch = toupper(ch);
    }

    switch (ev->event.type) {
        case SDL_KEYDOWN:
            ev->screen->dirty = true;
            ev->screen->key_pressed = true;
            ev->screen->last_key = ch;
            break;

        case SDL_KEYUP:
            // Note we do not erase the last_key value.
            ev->screen->key_pressed = false;

            if (ch == SDLK_ESCAPE) {
                ev->screen->should_exit = true;
            }

            break;

        default:
            break;
    }
}
