/*
 * vm_segment.c
 *
 * The functions here allow you to allocate generic blocks of memory (or
 * "segments") for use anywhere else in the software. They can be used
 * to represent machine memory, removable media (like floppy disks),
 * etc.
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

    // We should NULL out memory and make explicit that any new segment
    // begins life in that state.
    memset(segment->memory, (int)NULL, sizeof(vm_8bit));

    segment->read_table = malloc(sizeof(vm_segment_read_fn) * size);
    if (segment->read_table == NULL) {
        log_critical("Couldn't allocate enough space for segment read_table");
        return NULL;
    }

    segment->write_table = malloc(sizeof(vm_segment_write_fn) * size);
    if (segment->write_table == NULL) {
        log_critical("Couldn't allocate enough space for segment write_table");
        return NULL;
    }

    /*
     * Let's NULL-out the read and write tables. If we don't do so, they
     * may have some bits of garbage in it, and could cause the
     * read/write mapper code to attempt to a run a function with
     * garbage. We could have undefined garbage! We can only properly
     * work with defined garbage.
     */
    memset(segment->read_table, (int)NULL, sizeof(vm_segment_read_fn));
    memset(segment->write_table, (int)NULL, sizeof(vm_segment_write_fn));

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
int
vm_segment_set(vm_segment *segment, size_t index, vm_8bit value)
{
    // Some bounds checking.
    if (!vm_segment_bounds_check(segment, index)) {
        log_critical(
            "Attempt to set segment index (%d) greater than bounds (%d)",
            index,
            segment->size);

        return ERR_OOB;
    }

    segment->memory[index] = value;
    return OK;
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
int
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

        return ERR_OOB;
    }

    if (dest_index + length >= dest->size) {
        log_critical(
            "Attempt to copy beyond bounds of vm_segment (%d + %d >= %d)",
            dest_index,
            length,
            dest->size);

        return ERR_OOB;
    }

    memcpy(dest->memory + dest_index, 
           src->memory + src_index, 
           length * sizeof(src->memory[src_index]));

    return OK;
}
