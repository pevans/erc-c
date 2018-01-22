/*
 * apple2.dbuf.c
 */

#include "apple2.dbuf.h"

static size_t switch_reads[] = {
    0xC01A,
    0xC01B,
    0xC01E,
    0xC01F,
    0xC050,
    0xC051,
    0xC052,
    0xC053,
    0xC05E,
    0xC05F,
    0xC07E,
    0xC07F,
};

static size_t switch_writes[] = {
    0xC00C,
    0xC00D,
    0xC00E,
    0xC00F,
    0xC050,
    0xC051,
    0xC052,
    0xC053,
    0xC05E,
    0xC05F,
    0xC07E,
    0xC07F,
};

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
    int i, rlen, wlen;

    for (addr = 0x400; addr < 0x800; addr++) {
        vm_segment_read_map(segment, addr, apple2_dbuf_read);
        vm_segment_write_map(segment, addr, apple2_dbuf_write);
    }

    for (addr = 0x2000; addr < 0x4000; addr++) {
        vm_segment_read_map(segment, addr, apple2_dbuf_read);
        vm_segment_write_map(segment, addr, apple2_dbuf_write);
    }

    rlen = sizeof(switch_reads) / sizeof(size_t);
    wlen = sizeof(switch_writes) / sizeof(size_t);

    for (i = 0; i < rlen; i++) {
        vm_segment_read_map(segment, switch_reads[i], apple2_dbuf_switch_read);
    }

    for (i = 0; i < wlen; i++) {
        vm_segment_write_map(segment, switch_writes[i], apple2_dbuf_switch_write);
    }
}

/*
 * Handle all read switches for display buffer code. Some switches
 * respond to either reads _or_ writes, so you may see some cases
 * duplicated in the write map.
 *
 * Additionally, a number of the display modes also count as memory
 * modes, and are handled in apple2.mem.c. Notably, some are HIRES,
 * PAGE2, and 80STORE.
 */
SEGMENT_READER(apple2_dbuf_switch_read)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        case 0xC01E:
            return mach->display_mode & DISPLAY_ALTCHAR
                ? 0x80
                : 0x00;

        case 0xC01F:
            return mach->display_mode & DISPLAY_80COL
                ? 0x80
                : 0x00;

        case 0xC01A:
            return mach->display_mode & DISPLAY_TEXT
                ? 0x80
                : 0x00;

        case 0xC01B:
            return mach->display_mode & DISPLAY_MIXED
                ? 0x80
                : 0x00;

        // NOTE: IOUDIS is the only bit that seems to share a write
        // address with a read address. We can certainly handle that,
        // since we support separate read and write tables! But it's
        // interesting that they chose to do that, where they mostly
        // stick with separate addresses for separate functions.
        case 0xC07E:
            return mach->display_mode & DISPLAY_IOUDIS
                ? 0x80
                : 0x00;

        case 0xC07F:
            return mach->display_mode & DISPLAY_DHIRES
                ? 0x80
                : 0x00;

        // As in apple2.mem.c, the following switch cases are duplicated
        // from the switch_write function because the Apple II expects
        // both reads and writes to have the same effect at these
        // addresses.
        case 0xC050:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_TEXT);
            break;

        case 0xC051:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_TEXT);
            break;

        case 0xC052:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_MIXED);
            break;

        case 0xC053:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_MIXED);
            break;

        case 0xC05E:
            if (mach->display_mode & DISPLAY_IOUDIS) {
                apple2_set_display(mach,
                                   mach->display_mode | DISPLAY_DHIRES);
            }
            break;

        case 0xC05F:
            if (mach->display_mode & DISPLAY_IOUDIS) {
                apple2_set_display(mach,
                                   mach->display_mode & ~DISPLAY_DHIRES);
            }
            break;
    }

    // ???
    return 0;
}

/*
 * Here we update the statuses of the various display soft switches,
 * and--there's a bunch of them!
 */
SEGMENT_WRITER(apple2_dbuf_switch_write)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        case 0xC00E:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_ALTCHAR);
            break;

        case 0xC00F:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_ALTCHAR);
            break;

        case 0xC00C:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_80COL);
            break;

        case 0xC00D:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_80COL);
            break;

        case 0xC050:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_TEXT);
            break;

        case 0xC051:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_TEXT);
            break;

        case 0xC052:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_MIXED);
            break;

        case 0xC053:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_MIXED);
            break;

        case 0xC07E:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_IOUDIS);
            break;

        case 0xC07F:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_IOUDIS);
            break;

        case 0xC05E:
            apple2_set_display(mach,
                               mach->display_mode | DISPLAY_DHIRES);
            break;

        case 0xC05F:
            apple2_set_display(mach,
                               mach->display_mode & ~DISPLAY_DHIRES);
            break;
    }
}
