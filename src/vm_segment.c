#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "vm_segment.h"

static vm_segment *seg_table[VM_SEGMENT_TABLE_MAX];
static unsigned int seg_index = 0;

vm_segment *
vm_segment_create(size_t size)
{
    vm_segment *seg;

    // Block us from attempting to allocate any memory beyond the
    // maximum defined blocks.
    if (seg_index >= VM_SEGMENT_TABLE_MAX) {
        log_error("Attempted to allocate more segments than we allow");
        return NULL;
    }

    // Allocate memory for the current memory segment.
    seg = seg_table[seg_index] = 
        malloc(sizeof(vm_segment) * size);

    // Ack! We couldn't get the memory we wanted. Let's bail.
    if (seg == NULL) {
        log_critical("Couldn't allocate enough space for vm_segment");
        return NULL;
    }

    // We want to increment the current segment index only after a
    // _successful_ allocation.
    seg_index++;

    return seg;
}

void
vm_segment_set(vm_segment *segment, size_t index, vm_segment_byte value)
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

vm_segment_byte
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

void
vm_segment_copy(vm_segment *src, 
            vm_segment *dest, 
            size_t src_index, 
            size_t dest_index, 
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
