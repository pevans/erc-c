/*
 * vm_segment.c
 *
 * The functions here allow you to allocate generic blocks of memory (or
 * "segments") for use anywhere else in the software. They can be used
 * to represent machine memory, removable media (like floppy disks),
 * etc.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "vm_segment.h"

/*
 * This is sort of regrettable, but we need a machine pointer that we
 * can pass into the read/write map functions (which will assume to have
 * access to the machine architecture). The alternative is an update to
 * a lot more of the codebase to add machine pointers -- void pointers
 * at that -- which is even uglier.
 *
 * FIXME: we might consider a dependency injection container at some
 * point.
 */
static void *map_mach = NULL;

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
        segment->write_table[index](segment, index, value, map_mach);
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
            "Attempt to get segment index (%d) greater than bounds (%d)",
            index,
            segment->size);

        // See vm_segment_set() for a justification of this behavior.
        exit(1);
    }

    // We may have a read mapper for this address
    if (segment->read_table[index]) {
        return segment->read_table[index](segment, index, map_mach);
    }

    return segment->memory[index];
}

/*
 * Return a 16-bit value from a given address. This will read the byte
 * at addr and the byte at addr+1, then fit those into a two-byte
 * variable such that addr contains the most significant byte and addr+1
 * contains the least significant byte.
 */
vm_16bit
vm_segment_get16(vm_segment *segment, size_t addr)
{
    vm_16bit msb, lsb;

    lsb = (vm_16bit)vm_segment_get(segment, addr);
    msb = (vm_16bit)vm_segment_get(segment, addr+1);

    return (msb << 8) | lsb;
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
    if (src_index + length > src->size) {
        log_critical(
            "Attempt to copy beyond bounds of vm_segment (%d + %d >= %d)",
            src_index,
            length,
            src->size);

        return ERR_OOB;
    }

    if (dest_index + length > dest->size) {
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
 * Copy the contents of buf into the given dest segment. This is mostly
 * governed by the same restrictions that copy() has, except that we
 * can't do all of the bounds-checking we do there. This is just saying,
 * hey, I have a bunch of bytes and I just need this copied into a
 * segment, if you don't mind.
 */
int
vm_segment_copy_buf(vm_segment *dest, const vm_8bit *src, 
                    size_t destoff, size_t srcoff, size_t len)
{
    if (destoff + len > dest->size) {
        log_critical("Attempt to copy buffer out of bounds (%d + %d >= %d)",
                     destoff, len, dest->size);
        return ERR_OOB;
    }

    // Heh heh...there's no way of knowing if srcoff + len is out of
    // bounds at any point of src, since it's just a dumb buffer. Here's
    // hopin' it's not! Also, it'll be a fun day when sizeof(vm_8bit) is
    // not 1, BUT HEY. Let's do it right.
    memcpy(dest->memory + destoff, src + srcoff,
           len * sizeof(vm_8bit));

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
vm_segment_fread(vm_segment *segment, FILE *stream, size_t offset, size_t len)
{
    fread(segment->memory + offset, sizeof(vm_8bit), len, stream);

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

/*
 * Write the contents of a segment into a given file stream, using an
 * offset and a length for the contents to write. If this operation
 * works out, we return OK.
 */
int
vm_segment_fwrite(vm_segment *seg, FILE *stream, size_t off, size_t len)
{
    fwrite(seg->memory + off, sizeof(vm_8bit), len, stream);

    if (ferror(stream)) {
        log_critical("Could not write to the file stream: %s", strerror(errno));
        return ERR_BADFILE;
    }

    return OK;
}

/*
 * Change the internal notion of the machine used by map functions
 */
void
vm_segment_set_map_machine(void *mach)
{
    map_mach = mach;
}

/*
 * Return the map machine
 */
void *
vm_segment_get_map_machine()
{
    return map_mach;
}

/*
 * This is similar in spirit to the get16 function, but obviously more
 * practically similar to the set() function. Given a 16-bit value, we
 * will save this into the given address in little-endian order; this
 * therefore consumes bytes at both addr and addr+1.
 */
int
vm_segment_set16(vm_segment *segment, size_t addr, vm_16bit value)
{
    vm_8bit lsb, msb;
    int err;

    lsb = value & 0xff;
    msb = value >> 8;

    // This data needs to be saved in little-endian order; e.g. if we
    // get $1234, then we need to store it as $34 $12.
    err = vm_segment_set(segment, addr, lsb);

    // If the previous set() worked out, then let's try it again with
    // the msb.
    if (err == OK) {
        err = vm_segment_set(segment, addr + 1, msb);
    }

    // If err != OK above, we will just return the err code. If err was
    // OK, but is not OK after the msb set, then we'll return with that
    // code.
    return err;
}

/*
 * Print a hex dump of the region of memory between from and to into the
 * given file stream. This looks vaguely like the output of the hexdump
 * Unix command, except that the hex values are upper-case, and we use
 * brackets instead of vertical bars to delimit the ASCII output because
 * WHY NOT.
 */
void
vm_segment_hexdump(vm_segment *seg, FILE *stream, size_t from, size_t to)
{
    char nbuf[51], sbuf[17];
    int ni = 0, si = 0;
    int bytes = 0;
    vm_8bit byte;

    while (from < to) {
        byte = vm_segment_get(seg, from);

        ni += sprintf(nbuf + ni, "%02X ", byte);
        si += sprintf(sbuf + si, "%c", isprint(byte) ? byte : '.');

        from++;
        bytes++;

        if (bytes == 8) {
            ni += sprintf(nbuf + ni, " ");
        }

        if (bytes >= 16 || from >= to) {
            fprintf(stream, "%08zX    %s  [%s]\n", from - bytes, nbuf, sbuf);
            bytes = 0;
            ni = 0; 
            si = 0;
        }
    }
}
