#include <criterion/criterion.h>

#include "vm_event.h"

static vm_event ev;
static vm_screen *scr;

static void
setup()
{
    scr = vm_screen_create();
    ev.screen = scr;
}

static void
teardown()
{
    vm_screen_free(scr);
    memset(&ev, 0, sizeof(vm_event));
}

TestSuite(vm_event, .init = setup, .fini = teardown);

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
    ev.event.type = SDL_KEYDOWN;
    ev.event.key.keysym.sym = 'b';

    cr_assert_eq(scr->dirty, false);
    cr_assert_eq(scr->key_pressed, false);
    vm_event_keyboard(&ev);
}

Test(vm_event, keyboard_normal)
{
    ev.event.type = SDL_KEYDOWN;
    ev.event.key.keysym.sym = 'b';
    vm_event_keyboard(&ev);
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
}

Test(vm_event, keyboard_special)
{
    ev.event.type = SDL_KEYUP;
    cr_assert_eq(scr->should_exit, false);
    ev.event.key.keysym.sym = 'q';
    ev.event.key.keysym.mod = KMOD_ALT;
    vm_event_keyboard(&ev);
    cr_assert_eq(scr->should_exit, true);
}
