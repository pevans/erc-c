#include <criterion/criterion.h>

#include "apple2/apple2.h"
#include "apple2/dbuf.h"
#include "apple2/tests.h"
#include "vm_segment.h"

TestSuite(apple2_dbuf, .init = setup, .fini = teardown);

Test(apple2_dbuf, map)
{
    size_t addr;
    int i;
    vm_segment *segments[2];

    segments[0] = mach->main;
    segments[1] = mach->aux;
    for (i = 0; i < 2; i++) {
        for (addr = 0x400; addr < 0x800; addr++) {
            cr_assert_eq(segments[i]->read_table[addr], apple2_dbuf_read);
            cr_assert_eq(segments[i]->write_table[addr], apple2_dbuf_write);
        }

        for (addr = 0x2000; addr < 0x4000; addr++) {
            cr_assert_eq(segments[i]->read_table[addr], apple2_dbuf_read);
            cr_assert_eq(segments[i]->write_table[addr], apple2_dbuf_write);
        }
    }
}

/*
 * This test also works on apple2_dbuf_write.
 *
 * Test(apple2_dbuf, write)
 */
Test(apple2_dbuf, read)
{
    apple2_set_memory_mode(mach, MEMORY_80STORE | MEMORY_PAGE2);
    vm_segment_set(mach->main, 0x400, 123);
    vm_segment_set(mach->main, 0x2000, 234);
    cr_assert_neq(mach->main->memory[0x400], 123);
    cr_assert_eq(mach->aux->memory[0x400], 123);
    cr_assert_eq(mach->main->memory[0x2000], 234);
    cr_assert_neq(mach->aux->memory[0x2000], 234);

    apple2_set_memory_mode(mach, MEMORY_80STORE | MEMORY_PAGE2 | MEMORY_HIRES);
    vm_segment_set(mach->main, 0x2000, 234);
    cr_assert_eq(mach->main->memory[0x2000], 234);
    cr_assert_eq(mach->aux->memory[0x2000], 234);
}
