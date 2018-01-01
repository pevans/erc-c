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
        free(segment);
        log_critical("Couldn't allocate enough space for vm_segment");
        return NULL;
    }

    // We should zero out memory and make explicit that any new segment
    // begins life in that state.
    memset(segment->memory, 0, sizeof(vm_8bit) * size);

    segment->read_table = malloc(sizeof(vm_segment_read_fn) * size);
    if (segment->read_table == NULL) {
        log_critical("Couldn't allocate enough space for segment read_table");
        vm_segment_free(segment);
        return NULL;
    }

    segment->write_table = malloc(sizeof(vm_segment_write_fn) * size);
    if (segment->write_table == NULL) {
        log_critical("Couldn't allocate enough space for segment write_table");
        vm_segment_free(segment);
        return NULL;
    }

    // Let's NULL-out the read and write tables. If we don't do so, they
    // may have some bits of garbage in it, and could cause the
    // read/write mapper code to attempt to a run a function with
    // garbage. We could have undefined garbage! We can only properly
    // work with defined garbage.
    memset(segment->read_table, (int)NULL, sizeof(vm_segment_read_fn) * size);
    memset(segment->write_table, (int)NULL, sizeof(vm_segment_write_fn) * size);

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

    // Check if we have a write mapper
    if (segment->write_table[index]) {
        segment->write_table[index](segment, index, value);
        return OK;
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

    // We may have a read mapper for this address
    if (segment->read_table[index]) {
        return segment->read_table[index](segment, index);
    }

    return segment->memory[index];
}

/*
 * Copy a set of bytes from `src` (at `src_index`) to `dest` (at
 * `dest_index`), such that the range is `length` bytes long. Note that
 * this function presently bypasses our mapper function code... we may
 * need to implement such in the future.
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

/*
 * Set the read mapper for a given address. We'll use this function
 * instead of the normal logic on a get for that address.
 */
int
vm_segment_read_map(vm_segment *segment, 
                    size_t addr, 
                    vm_segment_read_fn fn)
{
    if (addr >= segment->size) {
        return ERR_OOB;
    }

    segment->read_table[addr] = fn;
    return OK;
}

/*
 * Here we set the map function for a given address to use on writes,
 * which is to say, when we use the `vm_segment_set()` function.
 */
int
vm_segment_write_map(vm_segment *segment,
                     size_t addr,
                     vm_segment_write_fn fn)
{
    if (addr >= segment->size) {
        return ERR_OOB;
    }

    segment->write_table[addr] = fn;
    return OK;
}

/*
 * Read the given file stream and write the contents into the given
 * segment, up to len bytes. If we could not read from the file stream
 * for some reason, signal that and return an error.
 */
int
vm_segment_fread(vm_segment *segment, FILE *stream, size_t len)
{
    fread(segment->memory, sizeof(vm_8bit), len, stream);

    // fread() may return zero in the case of an error, but it may
    // return a positive non-zero number short of len; we can't quite
    // count on just that to tell us something went wrong (especially if
    // len was not a valid length for the file to begin with).
    if (ferror(stream)) {
        log_critical("Could not read file stream: %s\n", strerror(errno));
        return ERR_BADFILE;
    }

    return OK;
}
