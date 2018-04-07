#ifndef _APPLE2_BANK_H_
#define _APPLE2_BANK_H_

#include "apple2/mem.h"
#include "vm_segment.h"

extern SEGMENT_READER(apple2_bank_read);
extern SEGMENT_READER(apple2_bank_switch_read);
extern SEGMENT_WRITER(apple2_bank_switch_write);
extern SEGMENT_WRITER(apple2_bank_write);
extern void apple2_bank_map(vm_segment *);

#endif
