#include <criterion/criterion.h>

#include "apple2.h"

static apple2 *mach;

void
setup()
{
    mach = apple2_create();
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
