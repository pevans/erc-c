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
vm_8bit
apple2_mem_read_bank(vm_segment *segment, size_t address, void *_mach)
{
    apple2 *mach;

    mach = (apple2 *)_mach;
    
    switch (mach->memory_mode) {
        // Return memory from the rom bank
        case MEMORY_BANK_ROM:
            // We need to account for the difference in address location
            // before we can successfully get any data from ROM.
            return vm_segment_get(mach->rom, address - APPLE2_BANK_OFFSET);

        // If the address is $D000..$DFFF, then we need to get the byte
        // from the ram2 bank. Otherwise, we break to use default
        // behavior.
        case MEMORY_BANK_RAM2:
            if (address < 0xE000) {
                // The same caution holds for getting data from the
                // second RAM bank.
                return vm_segment_get(mach->ram2, 
                                      address - APPLE2_BANK_OFFSET);
            }
            
            break;

        case MEMORY_BANK_RAM1:
        default:
            break;
    }

    // The "default" behavior as referred-to above is simply to return
    // the value as held in our primary memory bank.
    return segment->memory[address];
}

/*
 * Write a byte into bank-switchable memory. Many of the same cautions,
 * notes, etc. written for the read function apply here as well.
 */
void
apple2_mem_write_bank(vm_segment *segment, 
                      size_t address, vm_8bit value, void *_mach)
{
    apple2 *mach;

    mach = (apple2 *)_mach;

    switch (mach->memory_mode) {
        // Whoops -- we can't write any data into ROM.
        case MEMORY_BANK_ROM:
            return;

        case MEMORY_BANK_RAM2:
            if (address < 0xE000) {
                vm_segment_set(mach->ram2,
                               address - APPLE2_BANK_OFFSET, value);
                return;
            }

        case MEMORY_BANK_RAM1:
        default:
            break;
    }

    // Just set the value in main memory
    segment->memory[address] = value;
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
        vm_segment_read_map(mach->memory, addr, apple2_mem_read_bank);
        vm_segment_write_map(mach->memory, addr, apple2_mem_write_bank);
    }
}

/*
 * Since we can't write into ROM normally, we need a separate function
 * we can call which will do the writing for us.
 */
int
apple2_mem_init_disk2_rom(apple2 *mach)
{
    int err;
    vm_8bit *diskrom;

    diskrom = objstore_apple2_disk_rom();

    // Copy into the first peripheral page for disk ROM.
    err = vm_segment_copy_buf(mach->memory, diskrom,
                              APPLE2_DISK2_ROM_OFFSET, 0,
                              APPLE2_DISK2_ROM_SIZE);
    if (err != OK) {
        log_critical("Could not copy apple2 disk2 rom");
        return ERR_BADFILE;
    }

    // This second copy will copy into the next slot over, which is
    // essentially the second disk drive. It's all the same ROM.
    err = vm_segment_copy_buf(mach->memory, diskrom,
                              APPLE2_DISK2_ROM_OFFSET + 0x100, 0,
                              APPLE2_DISK2_ROM_SIZE);
    if (err != OK) {
        log_critical("Could not copy apple2 disk2 rom");
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
    vm_8bit *sysrom;
    
    sysrom = objstore_apple2_sys_rom();

    // The first two kilobytes of system rom are copied into memory
    // beginning at $C800 (which is just after all of the peripheral ROM
    // locations).
    err = vm_segment_copy_buf(mach->memory, sysrom,
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
