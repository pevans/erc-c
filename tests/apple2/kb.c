#include <criterion/criterion.h>

#include "apple2/kb.h"
#include "apple2/tests.h"

TestSuite(apple2_kb, .init = setup, .fini = teardown);

Test(apple2_kb, map)
{
    int i;
    vm_segment *segments[2];

    segments[0] = mach->main;
    segments[1] = mach->aux;

    for (i = 0; i < 2; i++) {
        cr_assert_eq(segments[i]->read_table[0xC000], apple2_kb_switch_read);
        cr_assert_eq(segments[i]->read_table[0xC010], apple2_kb_switch_read);
    }
}

Test(apple2_kb, switch_read)
{
    // Without strobe
    mach->screen->last_key = 'a';
    cr_assert_eq(vm_segment_get(mach->main, 0xC000), 'a');

    // With strobe
    mach->strobe = true;
    cr_assert_eq(vm_segment_get(mach->main, 0xC000), 'a' | 0x80);

    mach->screen->key_pressed = true;
    cr_assert_eq(vm_segment_get(mach->main, 0xC010), 0x80);
    cr_assert_eq(mach->strobe, false);

    mach->screen->key_pressed = false;
    cr_assert_eq(vm_segment_get(mach->main, 0xC010), 0x00);
}
