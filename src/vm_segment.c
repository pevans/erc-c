/*
 * vm_segment.c
 *   memory segments for our virtual machine
 *
 * Memory segments can be used for almost any kind of storage we can
 * imagine. The most obvious use would be for system memory, but others
 * would include physical disk media (floppy disks, hard drives).
 *
 * You may note that we assume memory segments are organized into 8bit
 * values (the `vm_8bit` type). It's certainly possible to create memory
 * segments which are organized into arbitrary boundaries; 2, 3, 4
 * bytes, you know, go nuts!
 *
 * To do so, however, we would be adding a fair bit of complexity, and
 * (at the moment of this writing at least) I am not convinced there is
 * a good use-case for doing so. Your bog-standard computer of _today_
 * is still using memory organized into bytes. Your hard drive is also
 * using bytes. Etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "vm_segment.h"

/*
 * Create a new segment, such that it contains a number of bytes indicated
 * by `size`. 
 */
vm_segment *
vm_segment_create(size_t size)
{
    vm_segment *segment;

    // Allocate memory for the current memory segment.
    segment = malloc(sizeof(vm_segment));

    // Ack! We couldn't get the memory we wanted. Let's bail.
    if (segment == NULL) {
        log_critical("Couldn't allocate enough space for vm_segment");
        return NULL;
    }

    segment->memory = malloc(sizeof(vm_8bit) * size);
    if (segment->memory == NULL) {
        log_critical("Couldn't allocate enough space for vm_segment");
        return NULL;
    }

    segment->size = size;

    return segment;
}

/*
 * Free the memory consumed by a given segment.
 */
void
vm_segment_free(vm_segment *segment)
{
    free(segment->memory);
    free(segment);
}

/*
 * Set the byte in `segment`, at `index`, to the given `value`. Our
 * bounds-checking here will _crash_ the program if we are
 * out-of-bounds.
 */
void
vm_segment_set(vm_segment *segment, size_t index, vm_8bit value)
{
    // Some bounds checking.
    if (!vm_segment_bounds_check(segment, index)) {
        log_critical(
            "Attempt to set segment index (%d) greater than bounds (%d)",
            index,
            segment->size);

        // We prefer to exit in this scenario, rather than try to
        // "handle" it with an overflow, because we would rather a crash
        // over ill-defined behavior.
        exit(1);
    }

    segment->memory[index] = value;
}

/*
 * Return the byte in `segment` at the given `index` point. Our
 * bounds-checking will _crash_ the program if an index is requested out
 * of bounds.
 */
vm_8bit
vm_segment_get(vm_segment *segment, size_t index)
{
    if (!vm_segment_bounds_check(segment, index)) {
        log_critical(
            "Attempt to set segment index (%d) greater than bounds (%d)",
            index,
            segment->size);

        // See vm_segment_set() for a justification of this behavior.
        exit(1);
    }

    return segment->memory[index];
}

/*
 * Copy a set of bytes from `src` (at `src_index`) to `dest` (at
 * `dest_index`), such that the range is `length` bytes long.
 */
void
vm_segment_copy(vm_segment *dest, 
            vm_segment *src, 
            size_t dest_index, 
            size_t src_index, 
            size_t length)
{
    if (src_index + length >= src->size) {
        log_critical(
            "Attempt to copy beyond bounds of vm_segment (%d + %d >= %d)",
            src_index,
            length,
            src->size);

        exit(1);
    }

    if (dest_index + length >= dest->size) {
        log_critical(
            "Attempt to copy beyond bounds of vm_segment (%d + %d >= %d)",
            dest_index,
            length,
            dest->size);

        exit(1);
    }

    memcpy(dest->memory + dest_index, 
           src->memory + src_index, 
           length * sizeof(src->memory[src_index]));
}
