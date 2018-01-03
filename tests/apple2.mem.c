#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.mem.h"

static apple2 *mach = NULL;

static void
setup()
{
    mach = apple2_create(100, 100);
    apple2_mem_map(mach);
}

static void
teardown()
{
    apple2_free(mach);
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
}

Test(apple2_mem, write_bank)
{
}
