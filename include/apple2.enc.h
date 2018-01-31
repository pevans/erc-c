#ifndef _APPLE2_ENC_H_
#define _APPLE2_ENC_H_

#include "vm_bits.h"
#include "vm_segment.h"

extern int apple2_enc_4n4(vm_segment *, int, vm_8bit); 
extern int apple2_enc_sector(vm_segment *, vm_segment *, int, int);
extern int apple2_enc_sector_header(vm_segment *, int, int, int);
extern int apple2_enc_track(vm_segment *, vm_segment *, int, int);
extern vm_segment *apple2_enc_dos(vm_segment *);

#endif
