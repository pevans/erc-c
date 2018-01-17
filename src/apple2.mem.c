/*
 * apple2.mem.c
 */

#include "apple2.bank.h"
#include "apple2.dbuf.h"
#include "apple2.pc.h"
#include "apple2.h"
#include "apple2.mem.h"
#include "objstore.h"

static size_t switch_reads[] = {
    0xC013,
    0xC014,
    0xC018,
    0xC01C,
    0xC01D,
};

static size_t switch_writes[] = {
    0xC000,
    0xC001,
    0xC002,
    0xC003,
    0xC004,
    0xC005,
    0xC054,
    0xC055,
    0xC056,
    0xC057,
    0xC059,
};

/*
 * Set the memory map functions for main memory in an apple2 machine
 */
void
apple2_mem_map(apple2 *mach, vm_segment *segment)
{
    size_t addr;
    int i, rlen, wlen;

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

    rlen = sizeof(switch_reads) / sizeof(size_t);
    wlen = sizeof(switch_writes) / sizeof(size_t);

    for (i = 0; i < rlen; i++) {
        vm_segment_read_map(segment, switch_reads[i], apple2_mem_switch_read);
    }

    for (i = 0; i < wlen; i++) {
        vm_segment_write_map(segment, switch_writes[i], apple2_mem_switch_write);
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

/*
 * Handle all soft switches that ask for the status of certain memory
 * conditions.
 */
SEGMENT_READER(apple2_mem_switch_read)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        case 0xC013:
            return mach->memory_mode & MEMORY_READ_AUX
                ? 0x80
                : 0x00;

        case 0xC014:
            return mach->memory_mode & MEMORY_WRITE_AUX
                ? 0x80
                : 0x00;

        case 0xC018:
            return mach->memory_mode & MEMORY_80STORE
                ? 0x80
                : 0x00;

        case 0xC01C:
            return mach->memory_mode & MEMORY_PAGE2
                ? 0x80
                : 0x00;

        case 0xC01D:
            return mach->memory_mode & MEMORY_HIRES
                ? 0x80
                : 0x00;
    }

    // ???
    return 0;
}

/*
 * Change memory settings based on the soft switches defined in the tech
 * reference
 */
SEGMENT_WRITER(apple2_mem_switch_write)
{
    apple2 *mach = (apple2 *)_mach;

    switch (addr) {
        case 0xC003:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_READ_AUX);
            break;

        case 0xC002:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_READ_AUX);
            break;

        case 0xC005:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_WRITE_AUX);
            break;

        case 0xC004:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_WRITE_AUX);
            break;

        case 0xC001:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_80STORE);
            break;

        case 0xC000:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_80STORE);
            break;

        case 0xC055:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_PAGE2);
            break;

        case 0xC054:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_PAGE2);
            break;

        // NOTE: in one place, the technical reference says $C057 is the
        // soft switch to enable HIRES. In one other place, it says
        // $C059. I'm assuming it's either one... It's otherwise
        // consistent when mentioning soft switches in more than one
        // table.
        case 0xC059:
        case 0xC057:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode | MEMORY_HIRES);
            break;

        case 0xC056:
            apple2_set_memory_mode(mach,
                                   mach->memory_mode & ~MEMORY_HIRES);
            break;

    }
}
