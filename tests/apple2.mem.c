#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.mem.h"

static apple2 *mach = NULL;

static void
setup()
{
    mach = apple2_create(100, 100);
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
        cr_assert_eq(mach->main->read_table[addr], apple2_mem_read_bank);
        cr_assert_eq(mach->main->write_table[addr], apple2_mem_write_bank);
    }
}

Test(apple2_mem, map_bank_switch)
{
    vm_segment *segments[2];

    segments[0] = mach->main;
    segments[1] = mach->aux;

    for (int i = 0; i < 2; i++) {
        cr_assert_eq(segments[i]->read_table[0xC080], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC081], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC082], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC083], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC088], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC089], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC08A], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC08B], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC088], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC011], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC012], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->read_table[0xC016], apple2_mem_read_bank_switch);
        cr_assert_eq(segments[i]->write_table[0xC008], apple2_mem_write_bank_switch);
        cr_assert_eq(segments[i]->write_table[0xC009], apple2_mem_write_bank_switch);
    }
}

Test(apple2_mem, read_bank)
{
    vm_8bit val;

    // Test that setting a value in the rom segment is returned to us
    // when addressing from main memory
    apple2_set_bank_switch(mach, MEMORY_ROM | MEMORY_WRITE);
    val = 123;
    vm_segment_set(mach->rom, 0x77, val);
    val = vm_segment_get(mach->rom, 0x77);
    cr_assert_eq(vm_segment_get(mach->main, 0xD077), val);

    // In RAM1 bank mode, setting a value in memory should return thaty
    // value in memory... but, as a twist, also check that the value is
    // not set in ROM nor in RAM2.
    val = 222;
    apple2_set_bank_switch(mach, MEMORY_WRITE);
    vm_segment_set(mach->main, 0xD077, val);
    cr_assert_eq(vm_segment_get(mach->main, 0xD077), val);
    cr_assert_neq(vm_segment_get(mach->rom, 0x77), val);
    cr_assert_neq(vm_segment_get(mach->main, 0x10077), val);

    // Finally, in RAM2 bank mode, we test similarly to ROM mode. Set
    // the value directly in ram2 (which is at $10000 - $1FFFF) and see
    // if it's there when addressing from main memory in the $Dnnn
    // range.
    val = 111;
    apple2_set_bank_switch(mach, mach->bank_switch | MEMORY_RAM2);
    vm_segment_set(mach->main, 0x10077, val);
    cr_assert_eq(vm_segment_get(mach->main, 0xD077), val);
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
    apple2_set_bank_switch(mach, MEMORY_ROM);
    vm_segment_set(mach->rom, 0x77, right);
    vm_segment_set(mach->main, 0xD077, wrong);
    cr_assert_eq(vm_segment_get(mach->rom, 0x77), right);
    cr_assert_eq(vm_segment_get(mach->main, 0xD077), right);

    // RAM1 is the main bank; it's all 64k RAM in one chunk.
    right = 111;
    wrong = 232;
    apple2_set_bank_switch(mach, MEMORY_WRITE);
    vm_segment_set(mach->main, 0xD078, right);
    vm_segment_set(mach->main, 0x10078, wrong);
    cr_assert_eq(vm_segment_get(mach->main, 0xD078), right);
    cr_assert_eq(vm_segment_get(mach->main, 0x10078), wrong);

    // RAM2 is most of the 64k, except the first 4k of the last 12
    // ($D000..$DFFF) is in ram2.
    right = 210;
    wrong = 132;
    apple2_set_bank_switch(mach, mach->bank_switch | MEMORY_RAM2);
    vm_segment_set(mach->main, 0x10073, wrong);
    vm_segment_set(mach->main, 0xD073, right);
    cr_assert_eq(vm_segment_get(mach->main, 0x10073), right);
}

Test(apple2_mem, read_bank_switch)
{
    vm_segment_get(mach->main, 0xC080);
    cr_assert_eq(mach->bank_switch, MEMORY_RAM2);

    // This (and a few others) are trickier to test, as they require
    // consecutive reads to trigger.
    vm_segment_get(mach->main, 0xC081);
    cr_assert_neq(mach->bank_switch, MEMORY_ROM | MEMORY_WRITE | MEMORY_RAM2);
    mach->cpu->last_addr = 0xC081;
    vm_segment_get(mach->main, 0xC081);
    cr_assert_eq(mach->bank_switch, MEMORY_ROM | MEMORY_WRITE | MEMORY_RAM2);

    vm_segment_get(mach->main, 0xC082);
    cr_assert_eq(mach->bank_switch, MEMORY_ROM | MEMORY_RAM2);

    // Another that needs consecutives
    vm_segment_get(mach->main, 0xC083);
    cr_assert_neq(mach->bank_switch, MEMORY_WRITE | MEMORY_RAM2);
    mach->cpu->last_addr = 0xC083;
    vm_segment_get(mach->main, 0xC083);
    cr_assert_eq(mach->bank_switch, MEMORY_WRITE | MEMORY_RAM2);

    vm_segment_get(mach->main, 0xC088);
    cr_assert_eq(mach->bank_switch, 0);

    vm_segment_get(mach->main, 0xC089);
    cr_assert_neq(mach->bank_switch, MEMORY_ROM | MEMORY_WRITE);
    mach->cpu->last_addr = 0xC089;
    vm_segment_get(mach->main, 0xC089);
    cr_assert_eq(mach->bank_switch, MEMORY_ROM | MEMORY_WRITE);

    vm_segment_get(mach->main, 0xC08A);
    cr_assert_eq(mach->bank_switch, MEMORY_ROM);

    vm_segment_get(mach->main, 0xC08B);
    cr_assert_neq(mach->bank_switch, MEMORY_WRITE);
    mach->cpu->last_addr = 0xC08B;
    vm_segment_get(mach->main, 0xC08B);
    cr_assert_eq(mach->bank_switch, MEMORY_WRITE);

    mach->bank_switch = MEMORY_RAM2;
    cr_assert_eq(vm_segment_get(mach->main, 0xC011), 0x80);
    mach->bank_switch = 0;
    cr_assert_eq(vm_segment_get(mach->main, 0xC011), 0x00);
    mach->bank_switch = 0;
    cr_assert_eq(vm_segment_get(mach->main, 0xC012), 0x80);
    mach->bank_switch = MEMORY_ROM;
    cr_assert_eq(vm_segment_get(mach->main, 0xC012), 0x00);
    mach->bank_switch = MEMORY_AUX;
    cr_assert_eq(vm_segment_get(mach->main, 0xC016), 0x80);
    mach->bank_switch = 0;
    cr_assert_eq(vm_segment_get(mach->main, 0xC016), 0x00);
}

Test(apple2_mem, write_bank_switch)
{
    vm_segment_set(mach->main, 0xC008, 1);
    cr_assert_eq(mach->bank_switch & MEMORY_AUX, MEMORY_AUX);
    vm_segment_set(mach->main, 0xC009, 1);
    cr_assert_eq(mach->bank_switch & MEMORY_AUX, 0);
}

Test(apple2_mem, init_peripheral_rom)
{
    // FIXME: this isn't working, _and_ it's pretty tightly coupled into
    // the create() function. We could use a better way of testing this.
    //cr_assert_eq(apple2_mem_init_disk2_rom(mach), OK);
}

Test(apple2_mem, init_sys_rom)
{
    // FIXME: same
    //cr_assert_eq(apple2_mem_init_sys_rom(mach), OK);
}
