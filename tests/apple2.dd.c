#include <criterion/criterion.h>

#include "apple2.dd.h"

static apple2dd *drive;

static void
setup()
{
    drive = apple2dd_create();
}

static void
teardown()
{
    apple2dd_free(drive);
}

TestSuite(apple2dd, .init = setup, .fini = teardown);

Test(apple2dd, create)
{
    cr_assert_eq(drive->data, NULL);
    cr_assert_eq(drive->track_pos, 0);
    cr_assert_eq(drive->sector_pos, 0);
    cr_assert_eq(drive->online, false);
    cr_assert_eq(drive->write_protect, true);
    cr_assert_eq(drive->mode, DD_READ);
}

Test(apple2dd, step)
{
    // Does step work at all?
    apple2dd_step(drive, 5);
    cr_assert_eq(drive->track_pos, 5);
    apple2dd_step(drive, 3);
    cr_assert_eq(drive->track_pos, 8);
    apple2dd_step(drive, -2);
    cr_assert_eq(drive->track_pos, 6);

    // Do we handle going over the maximum track position properly?
    apple2dd_step(drive, 100);
    cr_assert_eq(drive->track_pos, MAX_DRIVE_STEPS);

    // Do we handle going to the 0 track properly if we get a radically
    // high number of negative track shifts?
    apple2dd_step(drive, -1000);
    cr_assert_eq(drive->track_pos, 0);
}
