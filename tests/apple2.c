#include <criterion/criterion.h>

#include "apple2.h"
#include "mos6502.enums.h"
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

/* Test(apple2, free) */
/* Test(apple2, run_loop) */

Test(apple2, create)
{
    cr_assert_neq(mach, NULL);
    cr_assert_neq(mach->cpu, NULL);

    cr_assert_neq(mach->drive1, NULL);
    cr_assert_neq(mach->drive2, NULL);
}

Test(apple2, is_double_video)
{
    for (int i = 0; i <= VIDEO_DOUBLE_HIRES; i++) {
        mach->video_mode = i;
        switch (i) {
            case VIDEO_40COL_TEXT:
                cr_assert_eq(apple2_is_double_video(mach), false);
                break;
            case VIDEO_LORES:
                cr_assert_eq(apple2_is_double_video(mach), false);
                break;
            case VIDEO_HIRES:
                cr_assert_eq(apple2_is_double_video(mach), false);
                break;
            case VIDEO_80COL_TEXT:
                cr_assert_eq(apple2_is_double_video(mach), true);
                break;
            case VIDEO_DOUBLE_LORES:
                cr_assert_eq(apple2_is_double_video(mach), true);
                break;
            case VIDEO_DOUBLE_HIRES:
                cr_assert_eq(apple2_is_double_video(mach), true);
                break;
        }
    }
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

Test(apple2, press_key)
{
    apple2_press_key(mach, 123);
    cr_assert_eq(vm_segment_get(mach->main, 0xC000), 123 | 0x80);
    cr_assert_eq(vm_segment_get(mach->main, 0xC010), 0x80);
}

Test(apple2, clear_strobe)
{
    apple2_press_key(mach, 123);
    cr_assert_eq(vm_segment_get(mach->main, 0xC000), 123 | 0x80);
    apple2_clear_strobe(mach);
    cr_assert_eq(vm_segment_get(mach->main, 0xC000), 123);
}

/*
 * This also tests the press_key function (as does the clear_strobe
 * test, in its own way).
 */
Test(apple2, release_key)
{
    apple2_press_key(mach, 123);
    cr_assert_eq(vm_segment_get(mach->main, 0xC010), 0x80);
    apple2_release_key(mach);
    cr_assert_eq(vm_segment_get(mach->main, 0xC010), 0);
}

Test(apple2, set_color)
{
    apple2_set_color(mach, COLOR_AMBER);
    cr_assert_eq(mach->color_mode, COLOR_AMBER);
    apple2_set_color(mach, COLOR_FULL);
    cr_assert_eq(mach->color_mode, COLOR_FULL);
}

Test(apple2, set_video)
{
    apple2_set_video(mach, VIDEO_DOUBLE_HIRES);
    cr_assert_eq(mach->video_mode, VIDEO_DOUBLE_HIRES);
    apple2_set_video(mach, VIDEO_LORES);
    cr_assert_eq(mach->video_mode, VIDEO_LORES);
}

Test(apple2, set_bank_switch)
{
    apple2_set_bank_switch(mach, 0);
    cr_assert_eq(mach->bank_switch, 0);
    apple2_set_bank_switch(mach, BANK_WRITE | BANK_RAM2);
    cr_assert_eq(mach->bank_switch, BANK_WRITE | BANK_RAM2);

    mos6502_set(mach->cpu, 0x1, 111);
    mos6502_set(mach->cpu, 0x101, 222);

    apple2_set_bank_switch(mach, BANK_ALTZP);
    cr_assert_eq(mos6502_get(mach->cpu, 0x1), 111);
    cr_assert_eq(mos6502_get(mach->cpu, 0x101), 222);

    mos6502_set(mach->cpu, 0x1, 222);
    mos6502_set(mach->cpu, 0x101, 101);

    apple2_set_bank_switch(mach, BANK_DEFAULT);
    cr_assert_eq(mos6502_get(mach->cpu, 0x1), 222);
    cr_assert_eq(mos6502_get(mach->cpu, 0x101), 101);
}

Test(apple2, reset)
{
    vm_segment_set(mach->rom, 0x2FFC, 0x34);
    vm_segment_set(mach->rom, 0x2FFD, 0x12);
    apple2_reset(mach);

    cr_assert_eq(mach->cpu->PC, 0x1234);
    cr_assert_eq(mach->cpu->P, MOS_INTERRUPT);
    cr_assert_eq(mach->cpu->S, 0);
}

Test(apple2, set_memory_mode)
{
    apple2_set_memory_mode(mach, MEMORY_READ_AUX);
    cr_assert_eq(mach->memory_mode, MEMORY_READ_AUX);
    cr_assert_eq(mach->cpu->rmem, mach->aux);
    cr_assert_eq(mach->cpu->wmem, mach->main);

    apple2_set_memory_mode(mach, MEMORY_WRITE_AUX);
    cr_assert_eq(mach->memory_mode, MEMORY_WRITE_AUX);
    cr_assert_eq(mach->cpu->rmem, mach->main);
    cr_assert_eq(mach->cpu->wmem, mach->aux);
}
