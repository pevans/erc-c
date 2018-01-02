#ifndef _APPLE2_MEM_H_
#define _APPLE2_MEM_H_

#include "vm_segment.h"

extern vm_8bit apple2_mem_read_bank(vm_segment *, size_t);
extern void apple2_mem_write_bank(vm_segment *, size_t, vm_8bit);

#endif
