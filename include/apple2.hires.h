#ifndef _APPLE2_HIRES_H_
#define _APPLE2_HIRES_H_

#include "apple2.h"
#include "vm_bits.h"

extern int apple2_hires_update(size_t, int);
extern int apple2_hires_updated(size_t);
extern void apple2_hires_draw(apple2 *, int);
extern void apple2_hires_dump(apple2 *, FILE *);

#endif
