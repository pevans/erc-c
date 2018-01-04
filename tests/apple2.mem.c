#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.mem.h"

static apple2 *mach = NULL;

static void
setup()
{
    mach = apple2_create(100, 100);
    apple2_mem_map(mach);
    vm_segment_set_map_machine(mach);
}

static void
teardown()
{
    apple2_free(mach);
    vm_segment_set_map_machine(NULL);
}

TestSuite(apple2_mem, .init = setup, .fini = teardown);

Test(apple2_mem, map)
{
    // mach already had the mem_map function run, so we just need to
    // test the results.
    size_t addr;

    for (addr = APPLE2_BANK_OFFSET; addr < MOS6502_MEMSIZE; addr++) {
        cr_assert_eq(mach->memory->read_table[addr], apple2_mem_read_bank);
        cr_assert_eq(mach->memory->write_table[addr], apple2_mem_write_bank);
    }
}

Test(apple2_mem, read_bank)
{
    vm_8bit val;

    // Test that setting a value in the rom segment is returned to us
    // when addressing from main memory
    mach->memory_mode = MEMORY_BANK_ROM;
    val = 123;
    vm_segment_set(mach->rom, 0x77, val);
    val = vm_segment_get(mach->rom, 0x77);
    cr_assert_eq(vm_segment_get(mach->memory, 0xD077), val);

    // In RAM1 bank mode, setting a value in memory should return thaty
    // value in memory... but, as a twist, also check that the value is
    // not set in ROM nor in RAM2.
    val = 222;
    mach->memory_mode = MEMORY_BANK_RAM1;
    vm_segment_set(mach->memory, 0xD077, val);
    cr_assert_eq(vm_segment_get(mach->memory, 0xD077), val);
    cr_assert_neq(vm_segment_get(mach->rom, 0x77), val);
    cr_assert_neq(vm_segment_get(mach->ram2, 0x77), val);

    // Finally, in RAM2 bank mode, we test similarly to ROM mode. Set
    // the value directly in ram2 and see if it's there when addressing
    // from main memory.
    val = 111;
    mach->memory_mode = MEMORY_BANK_RAM2;
    vm_segment_set(mach->ram2, 0x77, val);
    cr_assert_eq(vm_segment_get(mach->memory, 0xD077), val);
}

Test(apple2_mem, write_bank)
{
}
