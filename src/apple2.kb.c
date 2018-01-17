/*
 * apple2.kb.c
 */

#include "apple2.kb.h"

/*
 * This mapper is considerably simpler than most, because it handles
 * only two addresses and only read maps. It merely performs the mapping
 * for keyboard switches.
 */
void
apple2_kb_map(vm_segment *seg)
{
    vm_segment_read_map(seg, 0xC000, apple2_kb_switch_read);
    vm_segment_read_map(seg, 0xC010, apple2_kb_switch_read);
}

/*
 * This mapper handles reads to switches that report info on the
 * keyboard state. Note that there is no write switch function; there
 * are only reads that need to be handled.
 */
SEGMENT_READER(apple2_kb_switch_read)
{
    apple2 *mach = (apple2 *)_mach;
    char ch = '\0';

    switch (addr) {
        case 0xC000:
            // This _also_ returns the 7 bit high if the strobe is still
            // set. Once you read from $C010, this bit will be cleared
            // in future reads--until another key is pressed.
            if (mach->screen) {
                ch = vm_screen_last_key(mach->screen);

                // If the strobe is set, we need to set the 7 bit on the
                // return value. (NOTE: Apple II can only handle 7-bit
                // ASCII, so the highest bit (bit 7, if counting from
                // zero) is always low in regards to ASCII
                // representation; it can _only_ be high if the strobe
                // is set.
                if (mach->strobe) {
                    return ch | 0x80;
                }
            }

            // If we have no screen, we'll just end up returning
            // whatever is in ch already (NUL).
            return ch;
            
        case 0xC010:
            // First, we need to clear the keyboard strobe.
            mach->strobe = false;

            if (mach->screen) {
                // Now we need to return whether any key was pressed, but
                // we'll have to ask the vm_screen subsystem. We just want
                // to return with the 7 bit high if it is pressed, and all
                // zeroes if not.
                return vm_screen_key_pressed(mach->screen)
                    ? 0x80
                    : 0x00;
            }

            // We have no keyboard, so we basically can have no key
            // pressed down.
            return 0;
    }

    // This can only happen if we were mapped to an address we weren't
    // prepared to handle
    return 0;
}
