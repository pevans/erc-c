#ifndef _APPLE2_H_
#define _APPLE2_H_

#include "mos6502.h"
#include "apple2.dd.h"

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
     * Our two disk drives.
     */
    apple2dd *drive1;
    apple2dd *drive2;
} apple2;

extern apple2 *apple2_create();
extern void apple2_free(apple2 *);
extern void apple2_press_key(apple2 *, vm_8bit);
extern void apple2_clear_strobe(apple2 *);
extern void apple2_release_key(apple2 *);
extern int apple2_boot(apple2 *);

#endif
