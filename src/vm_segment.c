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
#include "vm_di.h"
#include "vm_segment.h"

/*
 * Create a new segment, such that it contains a number of bytes indicated
 * by `size`. 
 */
vm_segment *
vm_segment_create(size_t size)
{
    vm_segment *seg;

    // Allocate memory for the current memory segment.
    seg = malloc(sizeof(vm_segment));

    // Ack! We couldn't get the memory we wanted. Let's bail.
    if (seg == NULL) {
        log_crit("Couldn't allocate enough space for vm_segment");
        return NULL;
    }

    seg->memory = malloc(sizeof(vm_8bit) * size);
    if (seg->memory == NULL) {
        free(seg);
        log_crit("Couldn't allocate enough space for vm_segment");
        return NULL;
    }

    // We should zero out memory and make explicit that any new segment
    // begins life in that state.
    memset(seg->memory, 0, sizeof(vm_8bit) * size);

    seg->read_table = malloc(sizeof(vm_segment_read_fn) * size);
    if (seg->read_table == NULL) {
        log_crit("Couldn't allocate enough space for segment read_table");
        vm_segment_free(seg);
        return NULL;
    }

    seg->write_table = malloc(sizeof(vm_segment_write_fn) * size);
    if (seg->write_table == NULL) {
        log_crit("Couldn't allocate enough space for segment write_table");
        vm_segment_free(seg);
        return NULL;
    }

    // Let's NULL-out the read and write tables. If we don't do so, they
    // may have some bits of garbage in it, and could cause the
    // read/write mapper code to attempt to a run a function with
    // garbage. We could have undefined garbage! We can only properly
    // work with defined garbage.
    memset(seg->read_table, (int)NULL, sizeof(vm_segment_read_fn) * size);
    memset(seg->write_table, (int)NULL, sizeof(vm_segment_write_fn) * size);

    seg->size = size;

    return seg;
}

/*
 * Free the memory consumed by a given segment.
 */
void
vm_segment_free(vm_segment *seg)
{
    free(seg->memory);
    free(seg);
}

/*
 * Set the byte in `segment`, at `addr`, to the given `value`. Our
 * bounds-checking here will _crash_ the program if we are
 * out-of-bounds.
 */
int
vm_segment_set(vm_segment *seg, size_t addr, vm_8bit value)
{
    // Some bounds checking.
    if (!vm_segment_bounds_check(seg, addr)) {
        log_crit(
            "Attempt to set segment addr (%d) greater than bounds (%d)",
            addr,
            seg->size);

        return ERR_OOB;
    }

    void *map_mach = vm_di_get(VM_MACHINE);

    // Check if we have a write mapper
    if (seg->write_table[addr]) {
        seg->write_table[addr](seg, addr, value, map_mach);
        return OK;
    }

    seg->memory[addr] = value;
    return OK;
}

/*
 * Return the byte in `segment` at the given `addr` point. Our
 * bounds-checking will _crash_ the program if an addr is requested out
 * of bounds.
 */
vm_8bit
vm_segment_get(vm_segment *seg, size_t addr)
{
    if (!vm_segment_bounds_check(seg, addr)) {
        log_crit(
            "Attempt to get segment addr (%d) greater than bounds (%d)",
            addr,
            seg->size);

        // See vm_segment_set() for a justification of this behavior.
        exit(1);
    }

    void *map_mach = vm_di_get(VM_MACHINE);

    // We may have a read mapper for this address
    if (seg->read_table[addr]) {
        return seg->read_table[addr](seg, addr, map_mach);
    }

    return seg->memory[addr];
}

/*
 * Return a 16-bit value from a given address. This will read the byte
 * at addr and the byte at addr+1, then fit those into a two-byte
 * variable such that addr contains the most significant byte and addr+1
 * contains the least significant byte.
 */
vm_16bit
vm_segment_get16(vm_segment *seg, size_t addr)
{
    vm_16bit msb, lsb;

    lsb = (vm_16bit)vm_segment_get(seg, addr);
    msb = (vm_16bit)vm_segment_get(seg, addr+1);

    return (msb << 8) | lsb;
}

/*
 * Copy a set of bytes from `src` (at `src_addr`) to `dest` (at
 * `dest_addr`), such that the range is `length` bytes long. Note that
 * this function presently bypasses our mapper function code... we may
 * need to implement such in the future.
 */
int
vm_segment_copy(vm_segment *dest, 
            vm_segment *src, 
            size_t dest_addr, 
            size_t src_addr, 
            size_t length)
{
    if (src_addr + length > src->size) {
        log_crit(
            "Attempt to copy beyond bounds of vm_segment (%d + %d >= %d)",
            src_addr,
            length,
            src->size);

        return ERR_OOB;
    }

    if (dest_addr + length > dest->size) {
        log_crit(
            "Attempt to copy beyond bounds of vm_segment (%d + %d >= %d)",
            dest_addr,
            length,
            dest->size);

        return ERR_OOB;
    }

    memcpy(dest->memory + dest_addr, 
           src->memory + src_addr, 
           length * sizeof(src->memory[src_addr]));

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
        log_crit("Attempt to copy buffer out of bounds (%d + %d >= %d)",
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
vm_segment_read_map(vm_segment *seg, 
                    size_t addr, 
                    vm_segment_read_fn fn)
{
    if (addr >= seg->size) {
        return ERR_OOB;
    }

    seg->read_table[addr] = fn;
    return OK;
}

/*
 * Here we set the map function for a given address to use on writes,
 * which is to say, when we use the `vm_segment_set()` function.
 */
int
vm_segment_write_map(vm_segment *seg,
                     size_t addr,
                     vm_segment_write_fn fn)
{
    if (addr >= seg->size) {
        return ERR_OOB;
    }

    seg->write_table[addr] = fn;
    return OK;
}

/*
 * Read the given file stream and write the contents into the given
 * segment, up to len bytes. If we could not read from the file stream
 * for some reason, signal that and return an error.
 */
int
vm_segment_fread(vm_segment *seg, FILE *stream, size_t offset, size_t len)
{
    fread(seg->memory + offset, sizeof(vm_8bit), len, stream);

    // fread() may return zero in the case of an error, but it may
    // return a positive non-zero number short of len; we can't quite
    // count on just that to tell us something went wrong (especially if
    // len was not a valid length for the file to begin with).
    if (ferror(stream)) {
        log_crit("Could not read file stream: %s\n", strerror(errno));
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
        log_crit("Could not write to the file stream: %s", strerror(errno));
        return ERR_BADFILE;
    }

    return OK;
}

/*
 * This is similar in spirit to the get16 function, but obviously more
 * practically similar to the set() function. Given a 16-bit value, we
 * will save this into the given address in little-endian order; this
 * therefore consumes bytes at both addr and addr+1.
 */
int
vm_segment_set16(vm_segment *seg, size_t addr, vm_16bit value)
{
    vm_8bit lsb, msb;
    int err;

    lsb = value & 0xff;
    msb = value >> 8;

    // This data needs to be saved in little-endian order; e.g. if we
    // get $1234, then we need to store it as $34 $12.
    err = vm_segment_set(seg, addr, lsb);

    // If the previous set() worked out, then let's try it again with
    // the msb.
    if (err == OK) {
        err = vm_segment_set(seg, addr + 1, msb);
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
