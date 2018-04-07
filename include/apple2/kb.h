#ifndef _APPLE2_KB_H_
#define _APPLE2_KB_H_

#include "apple2/apple2.h"
#include "vm_screen.h"
#include "vm_segment.h"

extern void apple2_kb_map(vm_segment *);
extern SEGMENT_READER(apple2_kb_switch_read);

#endif
