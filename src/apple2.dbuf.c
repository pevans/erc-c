/*
 * apple2.dbuf.c
 */

#include "apple2.dbuf.h"

/*
 * Handle reads from text page 1 and hires graphics page 1 (the display
 * buffers).
 */
SEGMENT_READER(apple2_dbuf_read)
{
    apple2 *mach = (apple2 *)_mach;

    // When 80STORE is high, we are allowed to look into two other flags
    // to see if they are set and modify our behavior accordingly.
    if (mach->memory_mode & MEMORY_80STORE) {
        // If the address is contained within text page 1, then we need
        // to look up memory in aux. (Uh, don't ask why we switch page 1
        // with the PAGE2 flag.) Otherwise, if the address is contained
        // in hires graphics page 1, then that must also use aux memory
        // if _both_ the PAGE2 and HIRES bits are set.
        if (addr >= 0x400 && addr < 0x800 && 
            mach->memory_mode & MEMORY_PAGE2
           ) {
            segment = mach->aux;
        } else if (addr >= 0x2000 && addr < 0x4000 &&
                   mach->memory_mode & MEMORY_PAGE2 & MEMORY_HIRES
                  ) {
            segment = mach->aux;
        }
    }

    // In all other cases, we _must_ use the segment that was passed in.
    // That segment may be aux memory if MEMORY_READ_AUX is set, and if
    // so, that would be the desired behavior.
    return segment->memory[addr];
}

/*
 * Many of the cautions and notes in the apple2_dbuf_read function
 * apply here as well.
 */
SEGMENT_WRITER(apple2_dbuf_write)
{
    apple2 *mach = (apple2 *)_mach;

    if (mach->memory_mode & MEMORY_80STORE) {
        if (addr >= 0x400 && addr < 0x800 && 
            mach->memory_mode & MEMORY_PAGE2
           ) {
            segment = mach->aux;
        } else if (addr >= 0x2000 && addr < 0x4000 &&
                   mach->memory_mode & MEMORY_PAGE2 &&
                   mach->memory_mode & MEMORY_HIRES
                  ) {
            segment = mach->aux;
        }
    }

    // Again, segment is allowed to be that which was passed in if
    // 80STORE is low. 
    segment->memory[addr] = value;
}

/*
 * Map the text page 1 and hires graphics page 1 addresses to the dbuf
 * read and write functions.
 */
void 
apple2_dbuf_map(vm_segment *segment)
{
    size_t addr;

    for (addr = 0x400; addr < 0x800; addr++) {
        vm_segment_read_map(segment, addr, apple2_dbuf_read);
        vm_segment_write_map(segment, addr, apple2_dbuf_write);
    }

    for (addr = 0x2000; addr < 0x4000; addr++) {
        vm_segment_read_map(segment, addr, apple2_dbuf_read);
        vm_segment_write_map(segment, addr, apple2_dbuf_write);
    }
}

/*
 * Handle all read switches for display buffer code. Some switches
 * respond to either reads _or_ writes, so you may see some cases
 * duplicated in the write map.
 */
SEGMENT_READER(apple2_dbuf_switch_read)
{
}
