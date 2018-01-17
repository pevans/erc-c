#include <criterion/criterion.h>

#include "apple2.bank.h"
#include "apple2.h"
#include "apple2.mem.h"
#include "apple2.tests.h"

TestSuite(apple2_mem, .init = setup, .fini = teardown);

Test(apple2_mem, map)
{
    size_t addr;
    int i;
    vm_segment *segments[2];

    segments[0] = mach->main;
    segments[1] = mach->aux;

    for (i = 0; i < 2; i++) {
        for (addr = 0x0; addr < 0x200; addr++) {
            cr_assert_eq(segments[i]->read_table[addr], apple2_mem_zp_read);
            cr_assert_eq(segments[i]->write_table[addr], apple2_mem_zp_write);
        }
    }
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

/*
 * You may notice some direct accesses to the memory field; it's needed
 * to get around the mapper functionality we're trying to test! This
 * test also works on both the read/write mapper functionality.
 *
 * Test(apple2_mem, zp_write)
 */
Test(apple2_mem, zp_read)
{
    apple2_set_bank_switch(mach, BANK_DEFAULT);
    mos6502_set(mach->cpu, 0, 123);
    cr_assert_eq(mach->main->memory[0], 123);
    cr_assert_neq(mach->aux->memory[0], 123);

    // Once we switch to BANK_ALTZP, we should see that the data in main
    // got copied over. That's tested elsewhere, but I put it here just
    // to make sure you have the right mental model when looking at this
    // test code.
    apple2_set_bank_switch(mach, BANK_ALTZP);
    cr_assert_eq(mach->aux->memory[0], 123);

    mos6502_set(mach->cpu, 0, 234);
    cr_assert_neq(mach->main->memory[0], 234);
    cr_assert_eq(mach->aux->memory[0], 234);
}
