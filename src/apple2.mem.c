/*
 * apple2.mem.c
 */

#include "apple2.bank.h"
#include "apple2.dbuf.h"
#include "apple2.pc.h"
#include "apple2.h"
#include "apple2.mem.h"
#include "objstore.h"

/*
 * Set the memory map functions for main memory in an apple2 machine
 */
void
apple2_mem_map(apple2 *mach, vm_segment *segment)
{
    size_t addr;

    vm_segment_set_map_machine(mach);

    // Set up all of the bank-switch-related mapping. Well--almost all
    // of it.
    apple2_bank_map(segment);

    // Here we handle the 80STORE bit for our display buffers.
    apple2_dbuf_map(segment);

    apple2_pc_map(segment);

    // We will do the mapping for the zero page and stack addresses.
    // Accessing those addresses can be affected by bank-switching, but
    // those addresses do not actually exist in the capital
    // Bank-Switching address space.
    for (addr = 0x0; addr < 0x200; addr++) {
        vm_segment_read_map(segment, addr, apple2_mem_zp_read);
        vm_segment_write_map(segment, addr, apple2_mem_zp_write);
    }
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
    const vm_8bit *prom;
    
    sysrom = objstore_apple2_sys_rom();
    prom = objstore_apple2_peripheral_rom();

    err = vm_segment_copy_buf(mach->rom, sysrom, 
                              0, 0, APPLE2_SYSROM_SIZE);
    if (err != OK) {
        log_critical("Could not copy apple2 system rom");
        return ERR_BADFILE;
    }

    err = vm_segment_copy_buf(mach->rom, prom,
                              APPLE2_SYSROM_SIZE, 0, 
                              APPLE2_PERIPHERAL_SIZE);
    if (err != OK) {
        log_critical("Could not copy apple2 peripheral rom");
        return ERR_BADFILE;
    }

    return OK;
}

/*
 * This is a wrapper for reads in the zero and stack pages; if
 * BANK_ALTZP is high, then we need to use aux memory regardless of the
 * segment passed into the function. Otherwise, we need to use main
 * memory--again, regardless of the passed-in segment.
 */
SEGMENT_READER(apple2_mem_zp_read)
{
    apple2 *mach = (apple2 *)_mach;

    // This is another case (see apple2.bank.c) where we don't care the
    // originating segment was; we only care whether BANK_ALTZP is on or
    // not. Like bank-switchable addresses, if it is on, then we should
    // be referencing aux memory; if not, main memory.
    segment = (mach->bank_switch & BANK_ALTZP)
        ? mach->aux
        : mach->main;

    return segment->memory[addr];
}

/*
 * This write function will intercept writes to the zero page and the
 * stack page; primarily as a wrapper to handle the BANK_ALTZP bit in
 * the bank_switch field of the apple2 struct.
 */
SEGMENT_WRITER(apple2_mem_zp_write)
{
    apple2 *mach = (apple2 *)_mach;

    // See the zp_read function for further details; the same logic
    // applies here.
    segment = (mach->bank_switch & BANK_ALTZP)
        ? mach->aux
        : mach->main;

    segment->memory[addr] = value;
}
