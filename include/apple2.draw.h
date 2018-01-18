#ifndef _APPLE2_DRAW_H_
#define _APPLE2_DRAW_H_

#include "apple2.h"
#include "vm_bits.h"

extern void apple2_draw_pixel(apple2 *, vm_16bit);
extern void apple2_draw_text(apple2 *, vm_16bit);
extern void apple2_draw_40col(apple2 *);

#endif
