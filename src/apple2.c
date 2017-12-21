/*
 * apple2.c
 *
 * Here we have support for the apple2 machine. I suspect that we will
 * need to break this file up into components in the future...
 */

#include "apple2.h"
#include "option.h"
#include "vm_segment.h"

/*
 * This is the memory address where an apple program can find the value
 * of the key that was last pressed.
 */
#define LAST_KEY 0xC000

/*
 * This is the address in memory where you can find whether a key is
 * currently pressed or not.
 */
#define ANY_KEY_DOWN 0xC010

/*
 * Create the basic apple2 structure.
 */
apple2 *
apple2_create(int width, int height)
{
    apple2 *mach;
    int err;

    mach = malloc(sizeof(apple2));
    if (mach == NULL) {
        return NULL;
    }

    mach->cpu = mos6502_create();
    mach->memory = mach->cpu->memory;

    // Our two drives -- we create both of them, even if we intend to
    // use only one.
    mach->drive1 = apple2dd_create();
    mach->drive2 = apple2dd_create();

    // Let's build our screen abstraction!
    mach->screen = vm_screen_create();
    if (mach->screen == NULL) {
        log_critical("Screen creation failed!\n");
        return NULL;
    }

    // We still need to add a window, since we want to render some
    // graphics.
    err = vm_screen_add_window(mach->screen, width, height);
    if (err != OK) {
        log_critical("Window creation failed!\n");
        return NULL;
    }

    // We default to lo-res mode.
    apple2_set_video(mach, VIDEO_LORES);

    return mach;
}

void
apple2_run_loop(apple2 *mach)
{
    while (vm_screen_active(mach->screen)) {
        vm_screen_set_color(mach->screen, 255, 0, 0, 255);
        vm_screen_draw_rect(mach->screen, 50, 50, 20, 20);
        vm_screen_refresh(mach->screen);
    }
}

/*
 * Free the memory reserved for an apple2 struct.
 */
void
apple2_free(apple2 *mach)
{
    mos6502_free(mach->cpu);

    // NOTE: we do _NOT_ want to clear the memory field of mach, as it's
    // co-owned with the cpu struct that we just freed above.

    free(mach);
}

void
apple2_press_key(apple2 *mach, vm_8bit ch)
{
    // The apple2 can only handle ASCII values of 0 through 127.
    // However, the eigth bit is called the "strobe" bit, and is treated
    // specially. In particular, the strobe bit is 1 if a key was
    // pressed down, and remains 1 until you reset it by reading from
    // the clear-strobe location.
    ch = ch | 0x80;

    // This is the location in memory where a program will expect to
    // find the value of the last key that was pressed.
    vm_segment_set(mach->memory, LAST_KEY, ch);

    // This area is a combination of flags; the eighth bit here is the
    // "any-key-down" flag, which is a bit of a mouthful. It's 1 if a
    // key is pressed, and 0 if not. The effect of reading this bit will
    // also _clear_ the strobe bit in the $C000 address (above).
    vm_segment_set(mach->memory, ANY_KEY_DOWN, 0x80);
}

/*
 * This function will clear the 8th bit, which is the "strobe" bit, from
 * the position in memory where the value of the last key that was
 * pressed is held.
 */
void
apple2_clear_strobe(apple2 *mach)
{
    vm_8bit ch;

    ch = vm_segment_get(mach->memory, LAST_KEY);
    vm_segment_set(mach->memory, LAST_KEY, ch & 0x7F);
}

/*
 * This function will clear the value of the any-key-down switch/flag.
 */
void
apple2_release_key(apple2 *mach)
{
    vm_segment_set(mach->memory, ANY_KEY_DOWN, 0);
}

int
apple2_boot(apple2 *mach)
{
    FILE *stream;
    int err;

    // Do we have any disks?
    stream = option_get_input(1);
    if (stream) {
        err = apple2dd_insert(mach->drive1, stream);
        if (err != OK) {
            log_critical("Unable to insert disk1 into drive");
            return err;
        }
    }

    stream = option_get_input(2);
    if (stream) {
        err = apple2dd_insert(mach->drive2, stream);
        if (err != OK) {
            log_critical("Unable to insert disk2 into drive");
            return err;
        }
    }

    return OK;
}

void
apple2_set_video(apple2 *mach, int mode)
{
    int width, height;

    mach->video_mode = mode;

    // In the traditional video modes that Apple II first came in, you
    // would have a maximum width of 280 pixels. (In lo-res, you have
    // fewer pixels, but that is something we have to handle in our
    // drawing functions rather than by changing the logical size.)
    width = 280;
    height = 192;

    // In double video modes, the width is effectively doubled, but the
    // height is untouched.
    if (mach->video_mode == VIDEO_DOUBLE_LORES ||
        mach->video_mode == VIDEO_DOUBLE_HIRES
       ) {
        width = 560;
    }

    vm_screen_set_logical_coords(mach->screen, width, height);
}
