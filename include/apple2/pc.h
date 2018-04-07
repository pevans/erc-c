#ifndef _APPLE2_PC_H_
#define _APPLE2_PC_H_

#include "apple2/apple2.h"
#include "vm_segment.h"

extern SEGMENT_READER(apple2_pc_read);
extern SEGMENT_READER(apple2_pc_switch_read);
extern SEGMENT_WRITER(apple2_pc_switch_write);
extern SEGMENT_WRITER(apple2_pc_write);
extern size_t apple2_pc_rom_addr(size_t, vm_8bit);
extern void apple2_pc_map(vm_segment *);

#endif
