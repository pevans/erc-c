/*
 * apple2.disk_drive.c
 */

#include "apple2.dd.h"

/*
 * This is the length of a typical disk that is formatted in either DOS
 * 3.3 or ProDOS.
 */
#define _140K_ 143360

/*
 * And this is the length of a disk that has been formatted as a nibble
 * file (*.NIB). This is not an Apple thing, exactly; it's more of an
 * emulator thing, that emulators had used to try and get around copy
 * protection in emulation. It does complicate disk drive operation!
 */
#define _240K_ 245760

/*
 * This is the last _accessible_ sector position within a track (you can
 * have 0 - 4095).
 */
#define MAX_SECTOR_POS 4095

apple2dd *
apple2dd_create()
{
    apple2dd *drive;

    drive = malloc(sizeof(apple2dd));
    if (drive == NULL) {
        log_critical("Could not malloc space for apple2 disk drive");
        return NULL;
    }

    // To begin with, we have no segment for data; that's something that
    // will depend on the disk you insert. For example, a DOS 3.3 or
    // ProDOS disk will have 140k, but a NIB file would have more.
    drive->data = NULL;

    drive->track_pos = 0;
    drive->sector_pos = 0;
    drive->online = false;
    drive->write_protect = true;
    drive->mode = DD_READ;

    return drive;
}

void
apple2dd_free(apple2dd *drive)
{
    if (drive->data) {
        vm_segment_free(drive->data);
    }

    free(drive);
}

void
apple2dd_step(apple2dd *drive, int steps)
{
    drive->track_pos += steps;

    if (drive->track_pos > MAX_DRIVE_STEPS) {
        drive->track_pos = MAX_DRIVE_STEPS;
    } else if (drive->track_pos < 0) {
        drive->track_pos = 0;
    }
}

void
apple2dd_set_mode(apple2dd *drive, int mode)
{
    if (mode != DD_READ && mode != DD_WRITE) {
        return;
    }

    drive->mode = mode;
}

void
apple2dd_turn_on(apple2dd *drive, bool online)
{
    drive->online = online;
}

void
apple2dd_write_protect(apple2dd *drive, bool protect)
{
    drive->write_protect = protect;
}

static int
position(apple2dd *drive)
{
    if (drive->data->size == _140K_) {
        int track_offset;

        track_offset = (drive->track_pos % 2) * 4096;
        return track_offset + drive->sector_pos;
    }

    return 0;
}

vm_8bit
apple2dd_read_byte(apple2dd *drive)
{
    vm_8bit byte = vm_segment_get(drive->data, position(drive));

    // We may have read the very last byte in a sector; if so let's
    // adjust the track_pos by two half tracks and reset the sector pos.
    drive->sector_pos++;
    if (drive->sector_pos > MAX_SECTOR_POS) {
        drive->track_pos += 2;
        drive->sector_pos = 0;
    }

    return byte;
}

void
apple2dd_write(apple2dd *drive, vm_8bit byte)
{
    vm_segment_set(drive->data, position(drive), byte);

    drive->sector_pos++;
    if (drive->sector_pos > MAX_SECTOR_POS) {
        drive->track_pos += 2;
        drive->sector_pos = 0;
    }
}
