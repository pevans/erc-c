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

int
apple2dd_insert(apple2dd *drive, FILE *stream)
{
    struct stat finfo;

    if (stream == NULL) {
        log_critical("File stream is null");
        return ERR_BADFILE;
    }

    // How large is this data set? Let's get the stat info.
    if (fstat(fileno(stream), &finfo)) {
        log_critical("Couldn't inspect file stream: %s", strerror(errno));
        return ERR_BADFILE;
    }

    if (finfo.st_size != _140K_) {
        log_critical("Unexpected file format (file size = %d)", finfo.st_size);
        return ERR_BADFILE;
    }

    // If we have any data, get rid of it. We'll start fresh here.
    apple2dd_eject(drive);

    drive->data = vm_segment_create(finfo.st_size);
    drive->track_pos = 0;
    drive->sector_pos = 0;

    return OK;
}

void
apple2dd_eject(apple2dd *drive)
{
    if (drive->data) {
        vm_segment_free(drive->data);
        drive->data = NULL;
    }
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

int
apple2dd_position(apple2dd *drive)
{
    // Special case: they didn't load any image data into the "drive".
    // Return zero.
    if (drive->data == NULL) {
        return 0;
    }

    // This is a normative DOS 3.3 / ProDOS disk. (Except ProDOS is
    // separated into 512 byte blocks which _shouldn't_ matter for our
    // purposes but let's not talk about that here do-de-doo.)
    if (drive->data->size == _140K_) {
        int track_offset;

        track_offset = (drive->track_pos % 2) * 4096;
        return track_offset + drive->sector_pos;
    }

    return 0;
}

void
apple2dd_shift(apple2dd *drive, int pos)
{
    drive->sector_pos += pos;

    if (drive->sector_pos > MAX_SECTOR_POS) {
        // We need to reset the sector pos to zero, because...
        drive->sector_pos = 0;
        
        // We also need to move to the next track, so let's adjust by
        // two half-tracks.
        apple2dd_step(drive, 2);
    }
}

vm_8bit
apple2dd_read_byte(apple2dd *drive)
{
    vm_8bit byte = vm_segment_get(drive->data, apple2dd_position(drive));
    apple2dd_shift(drive, 1);

    return byte;
}

void
apple2dd_write(apple2dd *drive, vm_8bit byte)
{
    vm_segment_set(drive->data, apple2dd_position(drive), byte);
    apple2dd_shift(drive, 1);
}
