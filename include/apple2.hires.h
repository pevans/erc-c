#ifndef _APPLE2_HIRES_H_
#define _APPLE2_HIRES_H_

#include "apple2.h"
#include "vm_bits.h"

extern vm_color apple2_hires_color_h0(vm_8bit);
extern vm_color apple2_hires_color_h1(vm_8bit);
extern void apple2_hires_draw(apple2 *, size_t);
extern int apple2_hires_row(size_t);
extern int apple2_hires_col(size_t);

#endif
