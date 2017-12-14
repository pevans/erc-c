/*
 * apple2.disk_drive.c
 */

#include "apple2.disk_drive.h"

apple2_disk_drive *
apple2_disk_drive_create()
{
    apple2_disk_drive *drive;

    drive = malloc(sizeof(apple2_disk_drive));
    if (drive == NULL) {
        log_critical("Could not malloc space for apple2 disk drive");
        return NULL;
    }

    // To begin with, we have no segment for data; that's something that
    // will depend on the disk you insert. For example, a DOS 3.3 or
    // ProDOS disk will have 140k, but a NIB file would have more.
    drive->data = NULL;

    drive->track_pos = 0;
    drive->online = false;
    drive->write_protect = true;
    drive->mode = DD_READ;

    return drive;
}

void
apple2_disk_drive_free(apple2_disk_drive *drive)
{
    if (drive->data) {
        vm_segment_free(drive->data);
    }

    free(drive);
}

void
apple2_disk_drive_step(apple2_disk_drive *drive, int steps)
{
    drive->track_pos += steps;

    if (drive->track_pos > MAX_DRIVE_STEPS) {
        drive->track_pos = MAX_DRIVE_STEPS;
    } else if (drive->track_pos < -MAX_DRIVE_STEPS) {
        drive->track_pos = -MAX_DRIVE_STEPS;
    }
}

void
apple2_disk_drive_set_mode(apple2_disk_drive *drive, int mode)
{
    if (mode != DD_READ && mode != DD_WRITE) {
        return;
    }

    drive->mode = mode;
}

void
apple2_disk_drive_turn_on(apple2_disk_drive *drive, bool online)
{
    drive->online = online;
}

void
apple2_disk_drive_write_protect(apple2_disk_drive *drive, bool protect)
{
    drive->write_protect = protect;
}
