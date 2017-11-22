#ifndef _VM_SEGMENT_H_
#define _VM_SEGMENT_H_

#define VM_SEGMENT_TABLE_MAX 16

typedef uint8_t vm_segment_byte;

typedef struct {
	size_t size;
	vm_segment_byte *memory;
} vm_segment;

extern void vm_segment_copy(vm_segment *, vm_segment *, size_t, size_t, size_t);
extern vm_segment *vm_segment_create(size_t);
extern vm_segment_byte vm_segment_get(vm_segment *, size_t);
extern void vm_segment_set(vm_segment *, size_t, vm_segment_byte);

#define vm_segment_bounds_check(segment, index) \
	(index == index % segment->size)

#endif
