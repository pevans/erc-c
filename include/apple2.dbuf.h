#ifndef _APPLE2_DBUF_H_
#define _APPLE2_DBUF_H_

#include "apple2.h"
#include "vm_segment.h"

extern SEGMENT_READER(apple2_dbuf_read);
extern SEGMENT_WRITER(apple2_dbuf_write);
extern void apple2_dbuf_map(vm_segment *);
extern SEGMENT_READER(apple2_dbuf_switch_read);
extern SEGMENT_WRITER(apple2_dbuf_switch_write);

#endif
