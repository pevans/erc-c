#ifndef _VM_SEGMENT_H_
#define _VM_SEGMENT_H_

#include "vm_bits.h"

typedef struct {
	size_t size;
	vm_8bit *memory;
} vm_segment;

extern void vm_segment_copy(vm_segment *, vm_segment *, size_t, size_t, size_t);
extern vm_segment *vm_segment_create(size_t);
extern void vm_segment_free(vm_segment *);
extern vm_8bit vm_segment_get(vm_segment *, size_t);
extern void vm_segment_set(vm_segment *, size_t, vm_8bit);

#define vm_segment_bounds_check(segment, index) \
	(index == index % segment->size)

#endif
