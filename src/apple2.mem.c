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
    
    // In the case of bank-switchable memory, BANK_ALTZP is the ultimate
    // arbitrator; if it's on, we have to use aux, and if not, we have
    // to use main. Whatever the segment was that was passed in will
    // turn out to be immaterial.
    segment = (mach->bank_switch & BANK_ALTZP) ? mach->aux : mach->main;

    if (~mach->bank_switch & BANK_RAM) {
        // We need to account for the difference in address location
        // before we can successfully get any data from ROM.
        return vm_segment_get(mach->rom, addr - APPLE2_BANK_OFFSET);
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
SEGMENT_WRITER(apple2_mem_write_bank)
{
    apple2 *mach;

    mach = (apple2 *)_mach;

    // No writes are allowed... sorry!
    if (~mach->bank_switch & BANK_WRITE) {
        return;
    }

    // See my spiel in the read bank mapper; the same applies here.
    segment = (mach->bank_switch & BANK_ALTZP) ? mach->aux : mach->main;

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
 * Set the memory map functions for main memory in an apple2 machine
 */
void
apple2_mem_map(apple2 *mach, vm_segment *segment)
{
    size_t addr;

    vm_segment_set_map_machine(mach);

    for (addr = APPLE2_BANK_OFFSET; addr < MOS6502_MEMSIZE; addr++) {
        vm_segment_read_map(segment, addr, apple2_mem_read_bank);
        vm_segment_write_map(segment, addr, apple2_mem_write_bank);
    }

    apple2_mem_map_bank_switch(segment);
}

/*
 * This function will establish all of the mapper functions to handle
 * the soft switches for memory bank-switching.
 */
void
apple2_mem_map_bank_switch(vm_segment *segment)
{
    vm_segment_read_map(segment, 0xC080, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC081, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC082, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC083, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC088, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC089, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC08A, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC08B, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC011, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC012, apple2_mem_read_bank_switch);
    vm_segment_read_map(segment, 0xC016, apple2_mem_read_bank_switch);
    vm_segment_write_map(segment, 0xC008, apple2_mem_write_bank_switch);
    vm_segment_write_map(segment, 0xC009, apple2_mem_write_bank_switch);
}

/*
 * Initialize the peripheral ROM ($C100 - $C7FF).
 */
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

/*
 * Handle reads to the soft switches that handle bank-switching. Note
 * that some of these "reads" actually modify how banks are switched
 * between ROM, RAM, or bank 2 RAM. Sorry about that -- it's just the
 * way it worked on the Apple II.
 */
SEGMENT_READER(apple2_mem_read_bank_switch)
{
    apple2 *mach;
    vm_16bit last_addr;

    mach = (apple2 *)_mach;

    // We need to know the last opcode and address, because some of our
    // soft switches require two consecutive reads
    mos6502_last_executed(mach->cpu, NULL, NULL, &last_addr);

    switch (addr) {
        // The $C080 - $C083 range all control memory access while using
        // bank 2 RAM for the $Dnnn range. Note that here and in the
        // $C088 range, the returns are zero; I'm not exactly sure
        // that's what they should be, but the purpose of reading from
        // these soft switches is not actually to read anything useful,
        // but simply to change the bank switch mode.
        case 0xC080:
            apple2_set_bank_switch(mach, BANK_RAM | BANK_RAM2);
            return 0;

        case 0xC081:
            if (last_addr == addr) {
                apple2_set_bank_switch(mach, BANK_WRITE | BANK_RAM2);
            }
            return 0;
        case 0xC082:
            apple2_set_bank_switch(mach, BANK_RAM2);
            return 0;

        case 0xC083:
            if (last_addr == addr) {
                apple2_set_bank_switch(mach, BANK_RAM | BANK_WRITE | BANK_RAM2);
            }
            return 0;

        // Conversely, the $C088 - $C08B range control memory access
        // while using bank 1 RAM.
        case 0xC088:
            apple2_set_bank_switch(mach, BANK_RAM);
            return 0;

        case 0xC089:
            if (last_addr == addr) {
                apple2_set_bank_switch(mach, BANK_WRITE);
            }
            return 0;

        case 0xC08A:
            apple2_set_bank_switch(mach, BANK_DEFAULT);
            return 0;

        case 0xC08B:
            if (last_addr == addr) {
                apple2_set_bank_switch(mach, BANK_RAM | BANK_WRITE);
            }
            return 0;

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

    log_critical("Bank switch read mapper called with an unexpected address: %x", addr);
    return 0;
}

/*
 * Handle writes to the soft switches that modify bank-switching
 * behavior.
 */
SEGMENT_WRITER(apple2_mem_write_bank_switch)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        // Turn on auxiliary memory for zero page + stack
        case 0xC008:
            apple2_set_bank_switch(mach,
                                   mach->bank_switch | BANK_ALTZP);
            return;

        // Disable auxiliary memory for zero page + stack
        case 0xC009:
            apple2_set_bank_switch(mach,
                                   mach->bank_switch & ~BANK_ALTZP);
            return;
    }

    log_critical("Bank switch write mapper called with an unexpected address: %x", addr);
}
