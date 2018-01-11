/*
 * apple2.mem.c
 */

#include "apple2.h"
#include "apple2.mem.h"
#include "objstore.h"

/*
 * Return a byte of memory from a bank-switchable address. This may be
 * from ROM, from main memory, or from the "extra" 4k bank of RAM.
 */
SEGMENT_READER(apple2_mem_read_bank)
{
    apple2 *mach;

    mach = (apple2 *)_mach;
    
    if (mach->bank_switch & MEMORY_ROM) {
        // We need to account for the difference in address location
        // before we can successfully get any data from ROM.
        return vm_segment_get(mach->rom, addr - APPLE2_BANK_OFFSET);
    }

    // Each memory bank (main or auxiliary) have an additional 4k of RAM
    // that you can access through bank-switching in the $D000 - $DFFF
    // range, which is actually held at the _end_ of memory beyond the
    // 64k mark.
    if (addr < 0xE000 && mach->bank_switch & MEMORY_RAM2) {
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
SEGMENT_WRITER(apple2_mem_write_bank)
{
    apple2 *mach;

    mach = (apple2 *)_mach;

    // No writes are allowed... sorry!
    if (~mach->bank_switch & MEMORY_WRITE) {
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
    if (addr < 0xE000 && mach->bank_switch & MEMORY_RAM2) {
        segment->memory[addr + 0x3000] = value;
        return;
    }

    // But if bank 2 RAM is not turned on, or the address is between
    // $E000 - $FFFF, then writes go to bank 1 RAM, which is our main
    // memory.
    segment->memory[addr] = value;
}

/*
 * Set the memory map functions for main memory in an apple2 machine
 */
void
apple2_mem_map(apple2 *mach)
{
    size_t addr;

    vm_segment_set_map_machine(mach);

    for (addr = APPLE2_BANK_OFFSET; addr < MOS6502_MEMSIZE; addr++) {
        vm_segment_read_map(mach->main, addr, apple2_mem_read_bank);
        vm_segment_write_map(mach->main, addr, apple2_mem_write_bank);
    }
}

int
apple2_mem_init_peripheral_rom(apple2 *mach)
{
    int err;

    // Let's copy beginning at the 1-slot offset in memory, but going
    // all the way as far as the length of all peripheral ROM in memory.
    err = vm_segment_copy_buf(mach->main, 
                              objstore_apple2_peripheral_rom(),
                              APPLE2_PERIPHERAL_SLOT(1), 0, 
                              APPLE2_PERIPHERAL_SIZE);
    if (err != OK) {
        log_critical("Could not copy apple2 peripheral rom");
        return ERR_BADFILE;
    }

    return OK;
}

/*
 * I'm still a bit hazy on how this _should_ work, but this function
 * will copy as much as we can from the system rom into both main memory
 * and into the rom segment.
 */
int
apple2_mem_init_sys_rom(apple2 *mach)
{
    int err;
    const vm_8bit *sysrom;
    
    sysrom = objstore_apple2_sys_rom();

    // The first two kilobytes of system rom are copied into memory
    // beginning at $C800 (which is just after all of the peripheral ROM
    // locations).
    err = vm_segment_copy_buf(mach->main, sysrom,
                              0xC800, 0x800, 0x800);
    if (err != OK) {
        log_critical("Could not copy apple2 system rom");
        return ERR_BADFILE;
    }

    // The last 12k of sysrom (which is APPLE2_ROM_SIZE) are copied into
    // the rom segment.
    err = vm_segment_copy_buf(mach->rom, sysrom, 
                              0, 0x1000, APPLE2_ROM_SIZE);
    if (err != OK) {
        log_critical("Could not copy apple2 system rom");
        return ERR_BADFILE;
    }

    return OK;
}
