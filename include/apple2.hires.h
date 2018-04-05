#ifndef _APPLE2_HIRES_H_
#define _APPLE2_HIRES_H_

#include "apple2.h"
#include "vm_bits.h"

enum hires_color {
    HIRES_GREEN,
    HIRES_PURPLE,
    HIRES_ORANGE,
    HIRES_BLUE,
    HIRES_BLACK,
    HIRES_WHITE,
};

extern void apple2_hires_draw(apple2 *, int);
extern void apple2_hires_dump(apple2 *, FILE *);

#endif
