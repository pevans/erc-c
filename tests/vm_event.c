#include <criterion/criterion.h>

#include "vm_event.h"

/*
 * There's not really much we can test here--right now--since almost
 * everything is driven by the SDL_PollEvent() function.
 */
Test(vm_event, poll)
{
}

/*
 * This is...quite a long test. We probably should break up the logic
 * for vm_event_keyboard soon!
 */
Test(vm_event, keyboard)
{
    vm_event ev;
    vm_screen *scr;

    scr = vm_screen_create();
    ev.screen = scr;
    
    ev.event.type = SDL_KEYDOWN;
    ev.event.key.keysym.sym = 'b';

    cr_assert_eq(scr->dirty, false);
    cr_assert_eq(scr->key_pressed, false);
    vm_event_keyboard(&ev);

    cr_assert_eq(scr->dirty, true);
    cr_assert_eq(scr->key_pressed, true);
    cr_assert_eq(scr->last_key, 'b');

    ev.event.key.keysym.mod = KMOD_LSHIFT;
    vm_event_keyboard(&ev);
    cr_assert_eq(scr->last_key, 'B');

    ev.event.key.keysym.sym = 'c';
    ev.event.key.keysym.mod = KMOD_RSHIFT;
    vm_event_keyboard(&ev);
    cr_assert_eq(scr->last_key, 'C');

    ev.event.type = SDL_KEYUP;
    vm_event_keyboard(&ev);
    cr_assert_eq(scr->key_pressed, false);

    cr_assert_eq(scr->should_exit, false);
    ev.event.key.keysym.sym = 'q';
    ev.event.key.keysym.mod = KMOD_ALT;
    vm_event_keyboard(&ev);
    cr_assert_eq(scr->should_exit, true);

    vm_screen_free(scr);
}
