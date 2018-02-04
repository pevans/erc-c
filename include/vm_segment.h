#ifndef _VM_SEGMENT_H_
#define _VM_SEGMENT_H_

#include "vm_bits.h"
#include "log.h"

/*
 * Here we make a forward declaration of the vm_segment. (We also
 * typedef that struct as `vm_segment`, so we don't have to say `struct
 * vm_segment` everywhere.)
 */
struct vm_segment;
typedef struct vm_segment vm_segment;

/*
 * The above forward declaration allows us to define the types for the
 * read and write function signatures that we intend to use in the
 * _definition_ of the vm_segment struct. 
 *
 * C is fun! Don't let anyone tell you otherwise.
 */
typedef vm_8bit (*vm_segment_read_fn)(vm_segment *, size_t, void *);
typedef void (*vm_segment_write_fn)(vm_segment *, size_t, vm_8bit, void *);

#define SEGMENT_READER(x) \
    vm_8bit x (vm_segment *segment, size_t addr, void *_mach)

#define SEGMENT_WRITER(x) \
    void x (vm_segment *segment, size_t addr, vm_8bit value, void *_mach)

/*
 * The bounds check is just some inline code to try and cut down on the
 * cost of it.
 */
#define vm_segment_bounds_check(segment, index) \
	(index == index % segment->size)

struct vm_segment {

    /*
     * The size of our memory segment. This is used for bounds checking.
     */
	size_t size;

    /*
     * This is the actual chunk of memory we allocate.
     */
	vm_8bit *memory;

    /*
     * These are memory maps; if we have defined a non-NULL entry for a
     * given address in read_table, then that is a function we will use
     * to return the value for that address. Likewise, if we have a
     * non-NULL entry in write_table, we will use that to "set" the
     * value. As you may guess, these should conventionally be the
     * exact size indicated by the size field.
     */
    vm_segment_read_fn *read_table;
    vm_segment_write_fn *write_table;
};

extern int vm_segment_copy(vm_segment *, vm_segment *, size_t, size_t, size_t);
extern int vm_segment_copy_buf(vm_segment *, const vm_8bit *, size_t, size_t, size_t);
extern int vm_segment_fread(vm_segment *, FILE *, size_t, size_t);
extern int vm_segment_fwrite(vm_segment *, FILE *, size_t, size_t);
extern int vm_segment_read_map(vm_segment *, size_t, vm_segment_read_fn);
extern int vm_segment_set(vm_segment *, size_t, vm_8bit);
extern int vm_segment_set16(vm_segment *, size_t, vm_16bit);
extern int vm_segment_write_map(vm_segment *, size_t, vm_segment_write_fn);
extern vm_16bit vm_segment_get16(vm_segment *, size_t);
extern vm_8bit vm_segment_get(vm_segment *, size_t);
extern vm_segment *vm_segment_create(size_t);
extern void *vm_segment_get_map_machine();
extern void vm_segment_free(vm_segment *);
extern void vm_segment_set_map_machine(void *);

#endif
