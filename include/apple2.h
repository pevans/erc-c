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

/*
 * The reset vector is the address where the apple will consult to
 * figure out where control should go after a reset. Think of this as
 * something like a pointer to a main() function in C. That is: where's
 * the main function? Let's ask the reset vector!
 */
#define APPLE2_RESET_VECTOR 0x03F2

/*
 * This is the address of the validity-check byte, aka the power-up
 * byte. The Apple II will use this to see if the reset vector is valid.
 */
#define APPLE2_POWERUP_BYTE 0x03F4

/*
 * I'm not _exactly_ clear on where the applesoft interpreter lives in
 * ROM, after spending possibly too-much time researching how this
 * works. My guess is I'm missing something that's obvious to others.
 * $E000 seems to be the original spot that Integer BASIC was contained,
 * and I'm going to guess Applesoft BASIC is in the same spot. Here's
 * hoping!
 */
#define APPLE2_APPLESOFT_MAIN 0xE000

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

// Write-protect on/off.
// Read target = ROM or RAM.
// Write target = RAM.
// Set mode of $Dxxx hexapage bank1 or bank2 ram.

// 0 - 0=off 1=on
// 1 - 0=ROM 1=RAM
// 2 - 0=BANK1 1=BANK2

/*
 * An Apple II has bank-switched memory beginning with $D000 extending
 * through $FFFF. The enums below define bit flag names to determine
 * what is accessible through those addresses.
 *
 * Note that it _is_ possible to write while reading ROM, but your
 * writes will not go to ROM; they'll go to _RAM_. Any write to $E000 -
 * $FFFF may only be sent to bank 1 RAM. Writes to $D000-$DFFF may
 * either be sent to bank 1 RAM or bank 2 RAM based upon the RAM2 bit
 * flag below.
 */
enum memory_mode {
    /*
     * By default, memory accesses go to main memory in _all_ cases.
     * Auxiliary memory is not used in any capacity.
     */
    MEMORY_DEFAULT = 0x0,

    /*
     * When this is on, the core 48k (non bank-switchable) of memory
     * will be read from auxiliary memory. When off, it will be read
     * from main memory.
     */
    MEMORY_READ_AUX = 0x1,
    
    /*
     * When on, writes to the core 48k of memory will go to aux; when
     * off, they go to main.
     */
    MEMORY_WRITE_AUX = 0x2,

    /*
     * This bit is what the tech reference calls an "enabling" switch,
     * for PAGE2 and HIRES below. If this bit is not on, then those two
     * other bits don't do anything, and all aux memory access is
     * governed by WRITE_AUX and READ_AUX above.
     */
    MEMORY_80STORE = 0x4,

    /*
     * When 80STORE is on, PAGE2 will allow you to access auxiliary
     * memory for the display page. The range depends on HIRES below.
     * When PAGE2 is on and HIRES is off, then PAGE2 causes accesses to
     * $0400..$07FF to always go to auxiliary memory (read or writes).
     * When both PAGE2 and HIRES are on, then $2000..$3FFF also go to
     * aux memory. When 80STORE is off, then these two bits are ignored.
     */
    MEMORY_PAGE2 = 0x8,
    MEMORY_HIRES = 0x10,
};

enum bank_switch {
    /*
     * In nominal bank-switch mode, reads in the bank-switchable address
     * space go to ROM; writes to RAM are protected; and bank2 memory is
     * used.
     */
    BANK_DEFAULT = 0x0,

    /* 
     * When on, this reads from RAM in bank-switched memory. When off,
     * it reads from ROM.
     */
    BANK_RAM = 0x1,
    
    /*
     * When on, we will write to RAM. When off, we will write-protect
     * RAM in bank-switched memory. NOTE: we can never write to ROM--or
     * else it wouldn't be ROM! So if you have BANK_RAM off, but
     * BANK_WRITE on, then writes do not fail, but they do go to RAM.
     */
    BANK_WRITE = 0x2,

    /*
     * When on, the $Dnnn hexapage will use the first RAM bank for
     * reads and/or writes. All other RAM access in bank-switched memory
     * will still go to bank 1 RAM, with respect to BANK_RAM when it
     * comes to reads. (The default behavior is actually to use bank 2
     * RAM for the $D range.)
     */
    BANK_RAM1 = 0x4,
    
    /*
     * This is a weird little bit. When BANK_ALTZP is on, the zero page
     * and stack are accessed from auxiliary memory rather than main
     * memory. Those two pages of memory, however,  are _copied_ from one
     * to the other, so data should remain consistent.
     *
     * That's not the weird part. That part makes sense given the name
     * (which isn't my name, but is the name used in the IIe technical
     * reference). The part that isn't so obvious is that
     * bank-switchable RAM will _also_ be accessed from auxiliary
     * memory, not main memory. Note that aux memory has its own second
     * bank of RAM, the way that main memory does, so BANK_RAM2 works
     * the way you think, but it works with the aux RAM2. No data is
     * copied between main and aux's bank-switched memory, unlike the
     * way zero page and the stack are handled.
     */
    BANK_ALTZP = 0x8,
};

typedef struct {
    /*
     * The apple 2 hardware used an MOS-6502 processor.
     */
    mos6502 *cpu;

    /*
     * This is the main memory bank of the computer. Conventionally, it
     * contains not only the first contiguous 48k of RAM, but it also
     * contains the last 12k of bank 1 RAM.
     */
    vm_segment *main;

    /*
     * The Apple II used a system of bank-switched memory to enable
     * software to address a separate block of ROM.
     */
    vm_segment *rom;
    
    /*
     * The Apple II may have an auxiliary RAM bank; this was possible by
     * installing a card there. If you had the 80-column text card (and
     * you likely did), then you got an extra kilobyte of RAM to work
     * with; it was either used for the extra columns or you could take
     * advantage of it for extra storage otherwise.
     */
    vm_segment *aux;

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
    vm_8bit bank_switch;

    /*
     * Beside bank-switching, we also need to keep track of memory
     * modes; these pertain mostly to reading from main or auxiliary
     * memory.
     */
    vm_8bit memory_mode;

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
extern void apple2_set_bank_switch(apple2 *, vm_8bit);
extern void apple2_set_video(apple2 *, int);

#endif
