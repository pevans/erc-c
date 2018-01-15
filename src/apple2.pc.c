/*
 * apple2.pc.c
 *
 * No, not "personal computer"; pc as in "peripheral card". The code
 * here is all about support for peripherals in general, excusing
 * support written for specific peripherals such as disk drives
 * (apple2.dd.c).
 */

#include "apple2.h"
#include "vm_segment.h"

/*
 * Map all of the peripheral ROM space (and peripheral expansion ROM
 * space) to our read and write mappers for that. For reference, normal
 * peripheral ROM space is $C100..$C7FF, and expansion ROM is
 * $C800..$CFFF.
 */
void
apple2_pc_map(apple2 *mach, vm_segment *seg)
{
    size_t addr;

    for (addr = 0xC100; addr < 0xD000; addr++) {
        vm_segment_read_map(seg, addr, apple2_pc_read);
        vm_segment_write_map(seg, addr, apple2_pc_write);
    }
}

SEGMENT_READER(apple2_pc_read)
{
    apple2 *mach = (apple2 *)_mach;

    if (mach->memory_mode & MEMORY_EXPROM) {
    }

    if (mach->memory_mode & MEMORY_SLOTCXROM) {
        segment = mach->rom;
    }
}
