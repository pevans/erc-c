/*
 * apple2.bank.c
 *
 * Handle reads and writes to bank-switchable memory, as well as the
 * soft switches that manage access to bank-switched memory spaces.
 * Bank-switchable memory is located from $D000..$FFFF, and those
 * addresses may point to 1) system ROM; 2) main memory RAM; 3)
 * auxiliary memory RAM; 4) a separate 4k bank of RAM in the
 * $D000..$DFFF range, one for _each_ of main and aux memory. That is,
 * you are allowed to fit a separate 16k RAM in 12k address space for
 * both main and auxiliary memory.
 *
 * Are you confused yet? Keep reading!
 */

#include "apple2/apple2.h"
#include "apple2/bank.h"
#include "apple2/mem.h"
#include "objstore.h"
#include "vm_di.h"
#include "vm_event.h"

/*
 * These are the addresses that need to be mapped to the
 * bank_switch_read function.
 */
static size_t switch_reads[] = {
    0xC011,
    0xC012,
    0xC016,
    0xC080,
    0xC081,
    0xC082,
    0xC083,
    0xC088,
    0xC089,
    0xC08A,
    0xC08B,
};

/*
 * These will be mapped to the bank_switch_write function.
 */
static size_t switch_writes[] = {
    0xC008,
    0xC009,
};

/*
 * Return a byte of memory from a bank-switchable address. This may be
 * from ROM, from main memory, or from the "extra" 4k bank of RAM.
 */
SEGMENT_READER(apple2_bank_read)
{
    apple2 *mach;

    mach = (apple2 *)_mach;
    
    if (~mach->bank_switch & BANK_RAM) {
        // We need to account for the difference in address location
        // before we can successfully get any data from ROM.
        return vm_segment_get(mach->rom, addr - APPLE2_SYSROM_OFFSET);
    }

    // Each memory bank (main or auxiliary) have an additional 4k of RAM
    // that you can access through bank-switching in the $D000 - $DFFF
    // range, which is actually held at the _end_ of memory beyond the
    // 64k mark.
    if (addr < 0xE000 && mach->bank_switch & BANK_RAM2) {
        // The same caution holds for getting data from the
        // second RAM bank.
        return segment->memory[addr + 0x3000];
    }

    // Otherwise, the byte is returned from bank 1 RAM, which is the
    // literal memory available in the segment.
    return segment->memory[addr];
}

/*
 * Write a byte into bank-switchable memory. Many of the same cautions,
 * notes, etc. written for the read function apply here as well.
 */
SEGMENT_WRITER(apple2_bank_write)
{
    apple2 *mach;

    mach = (apple2 *)_mach;

    // No writes are allowed... sorry!
    if (~mach->bank_switch & BANK_WRITE) {
        return;
    }

    // You will note, if we've gotten here, that it's possible to write
    // to the bank-switch addresses even if the ROM flag is 1. It's
    // true! Except that writes never go to ROM. That is to say, it's
    // possible to read from ROM and write to RAM at the same
    // time--well, nearly the same time, considering the 6502 does not
    // allow parallel actions!

    // In this case, we need to assign the value at the 64-68k range at
    // the end of memory; this is just a simple offset from the given
    // address.
    if (addr < 0xE000 && mach->bank_switch & BANK_RAM2) {
        segment->memory[addr + 0x3000] = value;
        return;
    }

    // But if bank 2 RAM is not turned on, or the address is between
    // $E000 - $FFFF, then writes go to bank 1 RAM, which is our main
    // memory.
    segment->memory[addr] = value;
}

/*
 * This function will establish all of the mapper functions to handle
 * the soft switches for memory bank-switching.
 */
void
apple2_bank_map(vm_segment *segment)
{
    size_t addr;
    int i, rlen, wlen;

    for (addr = APPLE2_BANK_OFFSET; addr < MOS6502_MEMSIZE; addr++) {
        vm_segment_read_map(segment, addr, apple2_bank_read);
        vm_segment_write_map(segment, addr, apple2_bank_write);
    }

    rlen = sizeof(switch_reads) / sizeof(size_t);
    wlen = sizeof(switch_writes) / sizeof(size_t);

    for (i = 0; i < rlen; i++) {
        vm_segment_read_map(segment, switch_reads[i],
                            apple2_bank_switch_read);
    }

    for (i = 0; i < wlen; i++) {
        vm_segment_write_map(segment, switch_writes[i], 
                             apple2_bank_switch_write);
    }
}

/*
 * Handle reads to the soft switches that handle bank-switching. Note
 * that some of these "reads" actually modify how banks are switched
 * between ROM, RAM, or bank 2 RAM. Sorry about that -- it's just the
 * way it worked on the Apple II.
 */
SEGMENT_READER(apple2_bank_switch_read)
{
    apple2 *mach;

    mach = (apple2 *)_mach;

    switch (addr) {
        // The $C080 - $C083 range all control memory access while using
        // bank 2 RAM for the $Dnnn range. Note that here and in the
        // $C088 range, the returns are zero; I'm not exactly sure
        // that's what they should be, but the purpose of reading from
        // these soft switches is not actually to read anything useful,
        // but simply to change the bank switch mode.
        case 0xC080:
            apple2_set_bank_switch(mach, BANK_RAM | BANK_RAM2);
            return 0x80;

        case 0xC081:
            apple2_set_bank_switch(mach, BANK_WRITE | BANK_RAM2);
            return 0x80;

        case 0xC082:
            apple2_set_bank_switch(mach, BANK_RAM2);
            return 0x80;

        case 0xC083:
            apple2_set_bank_switch(mach, BANK_RAM | BANK_WRITE | BANK_RAM2);
            return 0x80;

        // Conversely, the $C088 - $C08B range control memory access
        // while using bank 1 RAM.
        case 0xC088:
            apple2_set_bank_switch(mach, BANK_RAM);
            return 0x80;

        case 0xC089:
            apple2_set_bank_switch(mach, BANK_WRITE);
            return 0x80;

        case 0xC08A:
            apple2_set_bank_switch(mach, BANK_DEFAULT);
            return 0x80;

        case 0xC08B:
            apple2_set_bank_switch(mach, BANK_RAM | BANK_WRITE);
            return 0x80;

        // Return high on the 7th bit if we're using bank 2 memory
        case 0xC011:
            return mach->bank_switch & BANK_RAM2
                ? 0x80
                : 0x00;

        // Return high on 7th bit if we're reading RAM
        case 0xC012:
            return mach->bank_switch & BANK_RAM
                ? 0x80
                : 0x00;

        // Return high on the 7th bit if we are using the zero page and
        // stack from aux memory.
        case 0xC016:
            return mach->bank_switch & BANK_ALTZP
                ? 0x80
                : 0x00;
    }

    return 0;
}

/*
 * Handle writes to the soft switches that modify bank-switching
 * behavior.
 */
SEGMENT_WRITER(apple2_bank_switch_write)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        // Turn on auxiliary memory for zero page + stack
        case 0xC008:
            apple2_set_bank_switch(mach,
                                   mach->bank_switch & ~BANK_ALTZP);
            break;

        // Disable auxiliary memory for zero page + stack
        case 0xC009:
            apple2_set_bank_switch(mach,
                                   mach->bank_switch | BANK_ALTZP);
            break;
    }
}
