#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.pc.h"
#include "apple2.tests.h"

TestSuite(apple2_pc, .init = setup, .fini = teardown);

Test(apple2_pc, map)
{
    size_t addr;
    int i;
    vm_segment *segments[2];

    segments[0] = mach->main;
    segments[1] = mach->aux;

    for (i = 0; i < 2; i++) {
        for (addr = 0xC100; addr < 0xD000; addr++) {
            cr_assert_eq(segments[i]->read_table[addr], apple2_pc_read);
            cr_assert_eq(segments[i]->write_table[addr], apple2_pc_write);
        }

        cr_assert_eq(segments[i]->read_table[0xC015], apple2_pc_switch_read);
        cr_assert_eq(segments[i]->read_table[0xC017], apple2_pc_switch_read);
        cr_assert_eq(segments[i]->write_table[0xC00B], apple2_pc_switch_write);
        cr_assert_eq(segments[i]->write_table[0xC00A], apple2_pc_switch_write);
        cr_assert_eq(segments[i]->write_table[0xC006], apple2_pc_switch_write);
        cr_assert_eq(segments[i]->write_table[0xC007], apple2_pc_switch_write);
    }
}

Test(apple2_pc, rom_addr)
{
    cr_assert_eq(apple2_pc_rom_addr(0xC832, MEMORY_DEFAULT), 0x832);
    cr_assert_eq(apple2_pc_rom_addr(0xC832, MEMORY_SLOTCXROM), 0x832);
    cr_assert_eq(apple2_pc_rom_addr(0xC832, MEMORY_SLOTC3ROM), 0x832);
    cr_assert_eq(apple2_pc_rom_addr(0xC232, MEMORY_EXPROM), 0x0232);
    
    cr_assert_eq(apple2_pc_rom_addr(0xC832, MEMORY_EXPROM), 0x4832);
    cr_assert_eq(apple2_pc_rom_addr(0xC732, MEMORY_SLOTCXROM), 0x4732);
    cr_assert_eq(apple2_pc_rom_addr(0xC332, MEMORY_SLOTC3ROM), 0x4332);
}

/*
 * This function doesn't do _too_ much outside of calling rom_addr,
 * which we test elsewhere.
 */
Test(apple2_pc, read)
{
    vm_8bit rombyte;

    apple2_set_memory_mode(mach, MEMORY_DEFAULT);
    rombyte = vm_segment_get(mach->rom, 0x100);
    cr_assert_eq(vm_segment_get(mach->main, 0xC100), rombyte);
}

/*
 * The write map function should actually do nothing, since the memory
 * it works with is ROM. The test code is just looking to see if that is
 * the case.
 */
Test(apple2_pc, write)
{
    vm_8bit rombyte;

    rombyte = vm_segment_get(mach->rom, 0x100);
    vm_segment_set(mach->main, 0xC100, rombyte + 1);
    cr_assert_neq(vm_segment_get(mach->main, 0xC100), rombyte + 1);
}

Test(apple2_pc, switch_read)
{
    mach->memory_mode = MEMORY_DEFAULT;
    cr_assert_eq(vm_segment_get(mach->main, 0xC015), 0x80);
    mach->memory_mode = MEMORY_SLOTCXROM;
    cr_assert_eq(vm_segment_get(mach->main, 0xC015), 0);

    cr_assert_eq(vm_segment_get(mach->main, 0xC017), 0);
    mach->memory_mode = MEMORY_SLOTC3ROM;
    cr_assert_eq(vm_segment_get(mach->main, 0xC017), 0x80);
}

Test(apple2_pc, switch_write)
{
    mach->memory_mode = MEMORY_DEFAULT;
    vm_segment_set(mach->main, 0xC00B, 1);
    cr_assert_eq(mach->memory_mode, MEMORY_SLOTC3ROM);
    vm_segment_set(mach->main, 0xC00A, 1);
    cr_assert_eq(mach->memory_mode, MEMORY_DEFAULT);

    vm_segment_set(mach->main, 0xC006, 1);
    cr_assert_eq(mach->memory_mode, MEMORY_SLOTCXROM);
    vm_segment_set(mach->main, 0xC007, 1);
    cr_assert_eq(mach->memory_mode, MEMORY_DEFAULT);
}
