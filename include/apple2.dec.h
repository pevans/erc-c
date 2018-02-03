#ifndef _APPLE2_DEC_H_
#define _APPLE2_DEC_H_

#include "vm_segment.h"

extern int apple2_dec_dos(vm_segment *, vm_segment *);
extern int apple2_dec_nib(vm_segment *, vm_segment *);
extern int apple2_dec_sector(vm_segment *, vm_segment *, int, int);
extern int apple2_dec_track(vm_segment *, vm_segment *, int, int);

#endif
