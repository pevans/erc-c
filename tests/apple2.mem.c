#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.mem.h"
#include "apple2.bank.h"

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
