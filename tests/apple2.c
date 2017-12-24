#include <criterion/criterion.h>

#include "apple2.h"
#include "option.h"

static apple2 *mach;

void
setup()
{
    mach = apple2_create(700, 480);
}

void
teardown()
{
    apple2_free(mach);
}

TestSuite(apple2, .init = setup, .fini = teardown);

Test(apple2, create)
{
    cr_assert_neq(mach, NULL);
    cr_assert_neq(mach->cpu, NULL);

    cr_assert_neq(mach->drive1, NULL);
    cr_assert_neq(mach->drive2, NULL);
}

Test(apple2, press_key)
{
    apple2_press_key(mach, 123);
    cr_assert_eq(vm_segment_get(mach->memory, 0xC000), 123 | 0x80);
    cr_assert_eq(vm_segment_get(mach->memory, 0xC010), 0x80);
}

Test(apple2, clear_strobe)
{
    apple2_press_key(mach, 123);
    cr_assert_eq(vm_segment_get(mach->memory, 0xC000), 123 | 0x80);
    apple2_clear_strobe(mach);
    cr_assert_eq(vm_segment_get(mach->memory, 0xC000), 123);
}

Test(apple2, release_key)
{
    apple2_press_key(mach, 123);
    cr_assert_eq(vm_segment_get(mach->memory, 0xC010), 0x80);
    apple2_release_key(mach);
    cr_assert_eq(vm_segment_get(mach->memory, 0xC010), 0);
}

Test(apple2, boot)
{
    // A boot without any disks is technically ok... in full emulation,
    // you'd just see a screen that prints out something like "Apple
    // ][e" at the bottom.
    cr_assert_eq(apple2_boot(mach), OK);

    // And, as you may guess, it's ok to reboot the machine.
    option_read_file(1, "../data/zero.img");
    cr_assert_eq(apple2_boot(mach), OK);

    option_read_file(2, "../data/bad.img");
    cr_assert_neq(apple2_boot(mach), OK);
}
