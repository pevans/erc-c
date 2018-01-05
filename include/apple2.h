#ifndef _APPLE2_H_
#define _APPLE2_H_

#include "apple2.dd.h"
#include "mos6502.h"
#include "vm_bitfont.h"
#include "vm_screen.h"

/*
 * This is the size of the bitmap font we use for the apple2
 */
#define APPLE2_SYSFONT_SIZE 21558

enum video_modes {
    VIDEO_40COL_TEXT,
    VIDEO_LORES,
    VIDEO_HIRES,
    VIDEO_80COL_TEXT,
    VIDEO_DOUBLE_LORES,
    VIDEO_DOUBLE_HIRES,
};

enum color_modes {
    COLOR_GREEN,
    COLOR_AMBER,
    COLOR_GRAY,
    COLOR_FULL,
};

enum lores_colors {
    LORES_BLACK,
    LORES_MAGENTA,
    LORES_DARKBLUE,
    LORES_PURPLE,
    LORES_DARKGREEN,
    LORES_GRAY1,
    LORES_MEDBLUE,
    LORES_LIGHTBLUE,
    LORES_BROWN,
    LORES_ORANGE,
    LORES_GRAY2,
    LORES_PINK,
    LORES_LIGHTGREEN,
    LORES_YELLOW,
    LORES_AQUAMARINE,
    LORES_WHITE,
};

/*
 * These are the potential memory modes we understand. You can only have
 * one memory mode at a time.
 */
enum memory_mode {
    MEMORY_BANK_ROM,        // the last 12k is system ROM
    MEMORY_BANK_RAM1,       // the last 12k is system RAM
    MEMORY_BANK_RAM2,       // the first 4k of the last 12k is a separate RAM
                            // block from that in RAM1
};

typedef struct {
    /*
     * The apple 2 hardware used an MOS-6502 processor.
     */
    mos6502 *cpu;

    /*
     * This is the literal memory that the CPU above will create. You
     * should _not_ attempt to free this memory; allow the CPU's own
     * delete function to do that.
     */
    vm_segment *memory;

    /*
     * The Apple II used a system of bank-switched memory to enable
     * software to address a separate block of ROM.
     */
    vm_segment *rom;
    
    /*
     * Additionally, the Apple II had a standalone block of RAM (with no
     * good name for it, really, hence the regrettably vague "ram2") so
     * that you technically could use 16k of RAM from a set of 12k
     * addresses. The extra 4k lives a lonely life in the garage
     * apartment.
     */
    vm_segment *ram2;

    /*
     * The screen wherein we shall render all of our graphics.
     */
    vm_screen *screen;

    /*
     * This is the system font (the only font the Apple II knows about,
     * really); anywhere we render text, we have to use this font.
     */
    vm_bitfont *sysfont;

    /*
     * This is the mode in which we must interpret graphics. This will
     * tell us not only if we're in lo- or hi-res, but also if we are in
     * single or double view mode.
     */
    int video_mode;

    /*
     * This is the color mode we want to emulate. You can have a few
     * different styles of monochromatic displays: green, amber, and
     * light gray on black; you can also emulate a full color display,
     * in which text mode tends to look like light gray.
     */
    int color_mode;

    /*
     * This describes the behavior of our bank-switching scheme. We need
     * our read/write mappers to know where writes into the
     * bank-switched area of memory should target.
     */
    int memory_mode;

    /*
     * Our two disk drives.
     */
    apple2dd *drive1;
    apple2dd *drive2;
} apple2;

extern apple2 *apple2_create(int, int);
extern bool apple2_is_double_video(apple2 *);
extern int apple2_boot(apple2 *);
extern void apple2_clear_strobe(apple2 *);
extern void apple2_free(apple2 *);
extern void apple2_press_key(apple2 *, vm_8bit);
extern void apple2_release_key(apple2 *);
extern void apple2_reset(apple2 *);
extern void apple2_run_loop(apple2 *);
extern void apple2_set_color(apple2 *, int);
extern void apple2_set_video(apple2 *, int);

#endif
