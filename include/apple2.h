#ifndef _APPLE2_H_
#define _APPLE2_H_

#include "apple2.dd.h"
#include "mos6502.h"
#include "vm_bitfont.h"
#include "vm_screen.h"

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
     * Our two disk drives.
     */
    apple2dd *drive1;
    apple2dd *drive2;
} apple2;

extern apple2 *apple2_create(int, int);
extern void apple2_free(apple2 *);
extern void apple2_press_key(apple2 *, vm_8bit);
extern void apple2_clear_strobe(apple2 *);
extern void apple2_release_key(apple2 *);
extern int apple2_boot(apple2 *);
extern void apple2_run_loop(apple2 *);
extern void apple2_set_color(apple2 *, int);
extern void apple2_set_video(apple2 *, int);
extern bool apple2_is_double_video(apple2 *);

#endif
