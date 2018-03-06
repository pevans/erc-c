#ifndef _APPLE2_TEXT_H_
#define _APPLE2_TEXT_H_

#include "apple2.h"
#include "vm_bitfont.h"
#include "vm_bits.h"

extern char apple2_text_alternate(char);
extern char apple2_text_primary(char);
extern int apple2_text_area(vm_area *, vm_bitfont *, size_t);
extern void apple2_text_draw(apple2 *, size_t);

#endif
