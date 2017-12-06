#ifndef _APPLE2_H_
#define _APPLE2_H_

#include "mos6502.h"

typedef struct {
    /*
     * The apple 2 hardware used an MOS-6502 processor.
     */
    mos6502 *cpu;
} apple2;

extern apple2 *apple2_create();

#endif
