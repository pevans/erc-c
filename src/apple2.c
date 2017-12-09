/*
 * apple2.c
 *
 * Here we have support for the apple2 machine. I suspect that we will
 * need to break this file up into components in the future...
 */

#include "apple2.h"
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
apple2_create()
{
    apple2 *mach;

    mach = malloc(sizeof(apple2));
    if (mach == NULL) {
        return NULL;
    }

    mach->cpu = mos6502_create();
    mach->memory = mach->cpu->memory;
    
    return mach;
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
