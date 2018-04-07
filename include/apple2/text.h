#ifndef _APPLE2_TEXT_H_
#define _APPLE2_TEXT_H_

#include "apple2/apple2.h"
#include "vm_bitfont.h"
#include "vm_bits.h"

extern char apple2_text_alternate(vm_8bit);
extern char apple2_text_primary(vm_8bit);
extern int apple2_text_area(vm_area *, vm_bitfont *, size_t);
extern int apple2_text_col(size_t);
extern int apple2_text_row(size_t);
extern void apple2_text_draw(apple2 *, size_t);

#endif
