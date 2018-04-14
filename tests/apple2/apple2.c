#include <criterion/criterion.h>

#include "apple2/apple2.h"
#include "mos6502/enums.h"
#include "option.h"
#include "vm_di.h"

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
    mach->display_mode = DISPLAY_DEFAULT;
    cr_assert_eq(apple2_is_double_video(mach), false);
    mach->display_mode = DISPLAY_DHIRES;
    cr_assert_eq(apple2_is_double_video(mach), true);
}

Test(apple2, boot)
{
    // A boot without any disks is technically ok... in full emulation,
    // you'd just see a screen that prints out something like "Apple
    // ][e" at the bottom.
    cr_assert_eq(apple2_boot(mach), OK);

    FILE *stream1, *stream2;

    // And, as you may guess, it's ok to reboot the machine.
    option_open_file(&stream1, "../data/zero.img", "r");
    vm_di_set(VM_DISK1, stream1);

    cr_assert_eq(apple2_boot(mach), OK);

    option_open_file(&stream2, "../data/bad.img", "r");
    vm_di_set(VM_DISK2, stream2);
    cr_assert_neq(apple2_boot(mach), OK);

    fclose(stream1);
    fclose(stream2);

    vm_di_set(VM_DISK1, NULL);
    vm_di_set(VM_DISK2, NULL);
}

Test(apple2, set_color)
{
    apple2_set_color(mach, COLOR_AMBER);
    cr_assert_eq(mach->color_mode, COLOR_AMBER);
    apple2_set_color(mach, COLOR_FULL);
    cr_assert_eq(mach->color_mode, COLOR_FULL);
}

Test(apple2, set_display)
{
    apple2_set_display(mach, DISPLAY_DHIRES);
    cr_assert_eq(mach->display_mode, DISPLAY_DHIRES);
    apple2_set_display(mach, DISPLAY_TEXT);
    cr_assert_eq(mach->display_mode, DISPLAY_TEXT);
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
    vm_segment_set(mach->rom, 0x3FFC, 0x34);
    vm_segment_set(mach->rom, 0x3FFD, 0x12);
    apple2_reset(mach);

    cr_assert_eq(mach->cpu->PC, 0x1234);
    cr_assert_eq(mach->cpu->P, MOS_STATUS_DEFAULT);
    cr_assert_eq(mach->cpu->S, 0xff);
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

Test(apple2, notify_refresh)
{
    cr_assert_eq(mach->screen->dirty, false);
    apple2_notify_refresh(mach);
    cr_assert_eq(mach->screen->dirty, true);
}
