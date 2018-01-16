/*
 * apple2.pc.c
 *
 * No, not "personal computer"; pc as in "peripheral card". The code
 * here is all about support for peripherals in general, excusing
 * support written for specific peripherals such as disk drives
 * (apple2.dd.c).
 */

#include "apple2.pc.h"

static size_t switch_reads[] = {
    0xC015,
    0xC017,
};

static size_t switch_writes[] = {
    0xC006,
    0xC007,
    0xC00A,
    0xC00B,
};

/*
 * Map all of the peripheral ROM space (and peripheral expansion ROM
 * space) to our read and write mappers for that. For reference, normal
 * peripheral ROM space is $C100..$C7FF, and expansion ROM is
 * $C800..$CFFF.
 */
void
apple2_pc_map(vm_segment *seg)
{
    size_t addr;
    int i;
    int rlen, wlen;

    for (addr = 0xC100; addr < 0xD000; addr++) {
        vm_segment_read_map(seg, addr, apple2_pc_read);
        vm_segment_write_map(seg, addr, apple2_pc_write);
    }

    rlen = sizeof(switch_reads) / sizeof(size_t);
    wlen = sizeof(switch_writes) / sizeof(size_t);

    for (i = 0; i < rlen; i++) {
        vm_segment_read_map(seg, switch_reads[i], apple2_pc_switch_read);
    }

    for (i = 0; i < wlen; i++) {
        vm_segment_write_map(seg, switch_writes[i], apple2_pc_switch_write);
    }
}

/*
 * This mapper handles the entire potential peripheral ROM space; we
 * either need to return an address from the beginning of memory, or
 * near the end.
 */
SEGMENT_READER(apple2_pc_read)
{
    apple2 *mach = (apple2 *)_mach;

    // The address in the rom segment gets translated through a number
    // of factors
    addr = apple2_pc_rom_addr(addr, mach->memory_mode);

    // No matter what we do, the segment we return from will always be
    // the rom segment. This part is non-negotiable.
    return mach->rom->memory[addr];
}

/*
 * Since all of our address spaces are mapped to ROM, we can't write
 * anything!
 */
SEGMENT_WRITER(apple2_pc_write)
{
    return;
}
    

/*
 * Given an address from program code, return the corresponding address
 * in our rom segment. The machine's memory mode is also given as a
 * factor.
 */
size_t
apple2_pc_rom_addr(size_t addr, vm_8bit mode)
{
    size_t rom_addr;

    // This mapper should only be called from the $C000..$CFFF range;
    // in the rom segment, this group of pages is addressed at
    // $0000..$0FFF. We can assume that regardless of what addr is, a
    // subtraction of $C000 would result in a valid address.
    rom_addr = addr - 0xC000;

    // However, if the EXPROM bit is high, then we want to use expansion
    // ROM which is located at the _end_ of the rom segment; specifically
    // in the $4800..$4FFF range.
    if (rom_addr >= 0x0800 && rom_addr < 0x1000 &&
        mode & MEMORY_EXPROM
       ) {
        rom_addr += 0x4000;
    }

    // If SLOTCXROM is high, then we want any byte addressed in this
    // space to reference peripheral ROM (which is again located at the
    // end of the segment, in exactly the same way that expansion ROM is
    // placed).
    if (rom_addr >= 0x0100 && rom_addr < 0x0800 && 
        mode & MEMORY_SLOTCXROM
       ) {
        rom_addr += 0x4000;
    }

    // One final thing to account for is if the SLOTC3ROM bit is high.
    // If it is, and if the address has not already been incremented
    // because SLOTCXROM was _also_ high, then we need to increment just
    // for this specific page of memory.
    if (rom_addr >= 0x0300 && rom_addr < 0x0400 &&
        mode & MEMORY_SLOTC3ROM
       ) {
        rom_addr += 0x4000;
    }

    return rom_addr;
}

/*
 * Handle reads to the slot / peripheral ROM switches. I'm not
 * _entirely_ sure what Apple expects to be returned here, since they
 * don't indicate as clearly as they did with bank-switch modes; there,
 * they said you need to return a byte with the 7 bit high, which you
 * can then use a BPL or a BMI to branch with. Here, they don't specify
 * bit 7, but I'm going to go with that until I see otherwise.
 */
SEGMENT_READER(apple2_pc_switch_read)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        case 0xC015:
            return (mach->memory_mode & MEMORY_SLOTCXROM)
                ? 0x80
                : 0x00;
        case 0xC017:
            return (mach->memory_mode & MEMORY_SLOTC3ROM)
                ? 0x80
                : 0x00;
    }

    // We shouldn't get here for any practical reason, but...
    return 0;
}

/*
 * Handle writes to the slot switches. There are separate writes to turn
 * these switches on and off, for each switch, respectively. The value
 * being written doesn't matter (indeed--you won't see us reference the
 * value parameter).
 */
SEGMENT_WRITER(apple2_pc_switch_write)
{
    apple2 *mach = (apple2 *)_mach;

    // We don't care what the value is; if a write happens to any of the
    // following addresses, we must change the peripheral memory
    // behavior accordingly.
    switch (addr) {
        case 0xC00B:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_SLOTC3ROM);
            break;
        case 0xC00A:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_SLOTC3ROM);
            break;

        case 0xC006:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_SLOTCXROM);
            break;

        case 0xC007:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_SLOTCXROM);
            break;
    }
}
