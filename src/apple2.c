/*
 * apple2.c
 *
 * Here we have support for the apple2 machine. I suspect that we will
 * need to break this file up into components in the future...
 */

#include "apple2.h"
#include "apple2.draw.h"
#include "apple2.mem.h"
#include "mos6502.enums.h"
#include "mos6502.dis.h"
#include "objstore.h"
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

    objstore_init();

    mach = malloc(sizeof(apple2));
    if (mach == NULL) {
        return NULL;
    }

    // Forward set these to NULL in case we fail to build the machine
    // properly; that way, we won't try to free garbage data
    mach->rom = NULL;
    mach->ram2 = NULL;
    mach->sysfont = NULL;
    mach->screen = NULL;
    mach->drive1 = NULL;
    mach->drive2 = NULL;

    mach->cpu = mos6502_create();
    if (mach->cpu == NULL) {
        log_critical("Could not create CPU!");
        apple2_free(mach);
        return NULL;
    }

    // Our memory is that which is owned by the CPU.
    mach->memory = mach->cpu->memory;

    // Set the read/write mappers for everything
    apple2_mem_map(mach);

    // Initliaze our system ROM and separate bank-switched block of RAM
    mach->rom = vm_segment_create(APPLE2_ROM_SIZE);
    mach->ram2 = vm_segment_create(APPLE2_RAM2_SIZE);
    if (mach->rom == NULL || mach->ram2 == NULL) {
        log_critical("Could not initialize ROM / RAM2!");
        apple2_free(mach);
        return NULL;
    }

    if (apple2_mem_init_disk2_rom(mach) != OK) {
        log_critical("Could not initialize disk2 ROM");
        apple2_free(mach);
        return NULL;
    }

    if (apple2_mem_init_sys_rom(mach) != OK) {
        log_critical("Could not initialize apple2 ROM");
        apple2_free(mach);
        return NULL;
    }

    // Our two drives -- we create both of them, even if we intend to
    // use only one.
    mach->drive1 = apple2dd_create();
    mach->drive2 = apple2dd_create();

    if (mach->drive1 == NULL || mach->drive2 == NULL) {
        log_critical("Could not create disk drives!");
        apple2_free(mach);
        return NULL;
    }

    // Let's build our screen abstraction!
    mach->screen = vm_screen_create();
    if (mach->screen == NULL) {
        log_critical("Screen creation failed!");
        apple2_free(mach);
        return NULL;
    }

    // We still need to add a window, since we want to render some
    // graphics.
    err = vm_screen_add_window(mach->screen, width, height);
    if (err != OK) {
        log_critical("Window creation failed!");
        apple2_free(mach);
        return NULL;
    }

    // Default to full color
    apple2_set_color(mach, COLOR_FULL);

    // We default to lo-res mode.
    apple2_set_video(mach, VIDEO_LORES);

    // By default we should have ROM be the addressable last 12k of
    // memory.
    apple2_set_memory(mach, MEMORY_BANK_ROM);

    // Let's install our bitmap font.
    mach->sysfont = vm_bitfont_create(mach->screen,
                                      objstore_apple2_sysfont(),
                                      APPLE2_SYSFONT_SIZE,
                                      7, 8,         // 7 pixels wide, 8 pixels tall
                                      0x7f);        // 7-bit values only
    if (mach->sysfont == NULL) {
        apple2_free(mach);
        log_critical("Could not initialize apple2: bad font");
        return NULL;
    }

    return mach;
}

/*
 * Change the memory mode of the apple2 to a given mode.
 */
void
apple2_set_memory(apple2 *mach, int mode)
{
    mach->memory_mode = mode;
}

/*
 * Return true if we are in a state that the apple2 would consider
 * double resolution. (In practice, this refers to horizontal screen
 * density; vertical screen density per-pixel is unchanged.)
 */
bool
apple2_is_double_video(apple2 *mach)
{
    return 
        mach->video_mode == VIDEO_DOUBLE_HIRES ||
        mach->video_mode == VIDEO_DOUBLE_LORES ||
        mach->video_mode == VIDEO_80COL_TEXT;
}

/*
 * Try to "boot" the apple2 machine. Look for input sources indicated in
 * the option system and load those into our disk drives.
 */
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

    if (option_flag(OPTION_FLASH)) {
        mos6502_flash_memory(mach->cpu, mach->drive1->data);
    }

    if (option_flag(OPTION_DISASSEMBLE)) {
        mos6502_dis_scan(mach->cpu, stdout, 0, mach->cpu->memory->size);
    }

    // Run the reset routine to get the machine ready to go.
    apple2_reset(mach);

    return OK;
}

/*
 * This function marks out the procedures that happen when the machine
 * is reset. A reset can happen at a cold boot, but it can also happen
 * after the computer is already operational.
 */
void
apple2_reset(apple2 *mach)
{
    mach->cpu->P = MOS_INTERRUPT;
    mach->cpu->PC = vm_segment_get16(mach->memory, 0xFFFC);
    mach->cpu->S = 0;

    log_critical("At end of reset, PC = 0x%x", mach->cpu->PC);
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
 * Free the memory reserved for an apple2 struct.
 */
void
apple2_free(apple2 *mach)
{
    if (mach->cpu) {
        mos6502_free(mach->cpu);
    }

    if (mach->rom) {
        vm_segment_free(mach->rom);
    }

    if (mach->ram2) {
        vm_segment_free(mach->ram2);
    }

    if (mach->sysfont) {
        vm_bitfont_free(mach->sysfont);
    }

    if (mach->drive1) {
        apple2dd_free(mach->drive1);
    }

    if (mach->drive2) {
        apple2dd_free(mach->drive2);
    }

    if (mach->screen) {
        vm_screen_free(mach->screen);
    }

    // NOTE: we do _NOT_ want to clear the memory field of mach, as it's
    // co-owned with the cpu struct that we just freed above.

    free(mach);
}

/*
 * Emulate the notion of a pressed key in the apple2 with a given
 * character.
 */
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
 * This function will clear the value of the any-key-down switch/flag.
 */
void
apple2_release_key(apple2 *mach)
{
    vm_segment_set(mach->memory, ANY_KEY_DOWN, 0);
}

/*
 * The run loop is the function that essentially waits for user input
 * and continues to present the apple2 abstraction for you to use. At
 * some point the user will indicate they are done, whereby
 * vm_screen_active() will no longer be true and we exit.
 */
void
apple2_run_loop(apple2 *mach)
{
    if (option_flag(OPTION_DISASSEMBLE)) {
        return;
    }

    while (vm_screen_active(mach->screen)) {
        mos6502_dis_opcode(mach->cpu, stdout, mach->cpu->PC);
        mos6502_execute(mach->cpu,
                        vm_segment_get(mach->memory, mach->cpu->PC));
        vm_screen_refresh(mach->screen);
    }
}

/*
 * Set the color mode of the apple2, which is to say if we are emulating
 * a monochromatic display, or full color, or just black-and-white.
 */
void
apple2_set_color(apple2 *mach, int mode)
{
    mach->color_mode = mode;
    
    // FIXME: doing this should force us to redraw everything in the
    // correct color interpretation
}

/*
 * Set the video mode of the display. This would be the type of
 * resolution (text by which number of columns, lo-res, hi-res, etc.)
 */
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
