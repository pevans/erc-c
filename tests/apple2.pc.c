#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.pc.h"

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
