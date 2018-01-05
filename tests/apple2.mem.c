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

/*
 * The write_bank test will look a bit similar to the read_bank one,
 * except in logic it should be written somewhat as an inverse. That is,
 * we want our writes to all go to memory, and double-check that the
 * right location is being updated (or not being updated).
 */
Test(apple2_mem, write_bank)
{
    vm_8bit right, wrong;

    // In BANK_ROM mode, we expect that updates to ROM will never be
    // successful (after all, it wouldn't be read-only memory if they
    // were).
    right = 123;
    wrong = 222;
    mach->memory_mode = MEMORY_BANK_ROM;
    vm_segment_set(mach->rom, 0x77, right);
    vm_segment_set(mach->memory, 0xD077, wrong);
    cr_assert_eq(vm_segment_get(mach->rom, 0x77), right);
    cr_assert_eq(vm_segment_get(mach->memory, 0xD077), right);

    // RAM1 is the main bank; it's all 64k RAM in one chunk.
    right = 111;
    wrong = 232;
    mach->memory_mode = MEMORY_BANK_RAM1;
    vm_segment_set(mach->memory, 0xD078, right);
    vm_segment_set(mach->ram2, 0x78, wrong);
    cr_assert_eq(vm_segment_get(mach->memory, 0xD078), right);
    cr_assert_eq(vm_segment_get(mach->ram2, 0x78), wrong);

    // RAM2 is most of the 64k, except the first 4k of the last 12
    // ($D000..$DFFF) is in ram2.
    right = 210;
    wrong = 132;
    mach->memory_mode = MEMORY_BANK_RAM2;
    vm_segment_set(mach->ram2, 0x73, wrong);
    vm_segment_set(mach->memory, 0xD073, right);
    cr_assert_eq(vm_segment_get(mach->ram2, 0x73), right);
}
