/*
 * apple2.disk_drive.c
 */

#include "apple2.dd.h"

/*
 * Create a new disk drive. We do not create a memory segment for the
 * drive right away, as the size of said data can be variable based on
 * the disk format.
 */
apple2dd *
apple2_dd_create()
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

/*
 * Insert a "disk" into the drive, such that a disk is delivered to us
 * through a FILE stream. Return an error code if the disk format is
 * something we cannot accept.
 */
int
apple2_dd_insert(apple2dd *drive, FILE *stream)
{
    struct stat finfo;
    int err;

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
    apple2_dd_eject(drive);

    drive->data = vm_segment_create(finfo.st_size);
    drive->track_pos = 0;
    drive->sector_pos = 0;

    // Read the data from the stream and write into the memory segment
    err = vm_segment_fread(drive->data, stream, 0, finfo.st_size);
    if (err != OK) {
        log_critical("Could not read data into disk drive");
        return err;
    }

    return OK;
}

/*
 * Return the segment position that the drive is currently at, based
 * upon track and sector position.
 */
int
apple2_dd_position(apple2dd *drive)
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

/*
 * Read a single byte from the disk drive, at its current position, and
 * then shift the head by 1 byte.
 */
vm_8bit
apple2_dd_read(apple2dd *drive)
{
    vm_8bit byte = vm_segment_get(drive->data, apple2_dd_position(drive));
    apple2_dd_shift(drive, 1);

    return byte;
}

/*
 * Here we mean to "empty" the drive, essentially freeing the segment
 * memory and resetting the head position.
 */
void
apple2_dd_eject(apple2dd *drive)
{
    if (drive->data) {
        vm_segment_free(drive->data);
        drive->data = NULL;
    }

    drive->track_pos = 0;
    drive->sector_pos = 0;
}

/*
 * Free the memory taken up by the disk drive.
 */
void
apple2_dd_free(apple2dd *drive)
{
    if (drive->data) {
        vm_segment_free(drive->data);
    }

    free(drive);
}

/*
 * Set the disk drive mode, which is either read or write. (It can only
 * be one or the other at a time.)
 */
void
apple2_dd_set_mode(apple2dd *drive, int mode)
{
    if (mode != DD_READ && mode != DD_WRITE) {
        return;
    }

    drive->mode = mode;
}

/*
 * Shift the head position in the drive by the given positions, which is
 * in bytes. Pos may be a negative number; if so, the head essentially
 * moves further away from the center of the magnetic wafer.
 */
void
apple2_dd_shift(apple2dd *drive, int pos)
{
    drive->sector_pos += pos;

    while (drive->sector_pos > MAX_SECTOR_POS) {
        // We need to reset the sector pos to zero, because...
        drive->sector_pos -= (MAX_SECTOR_POS + 1);
        
        // We also need to move to the next track, so let's adjust by
        // two half-tracks.
        apple2_dd_step(drive, 2);
    }
}

/*
 * When you step the drive, you are essentially moving the head in
 * track positions. It's not really faster for _us_, but it's faster for
 * a mechanical drive than a bunch of shifts if you know the data is far
 * away track-wise. This function also safeguards (as the drive did!)
 * against stepping too far out or too far in.
 */
void
apple2_dd_step(apple2dd *drive, int steps)
{
    drive->track_pos += steps;

    if (drive->track_pos > MAX_DRIVE_STEPS) {
        drive->track_pos = MAX_DRIVE_STEPS;
    } else if (drive->track_pos < 0) {
        drive->track_pos = 0;
    }
}

/*
 * A really simple function to turn the drive "on".
 */
void
apple2_dd_turn_on(apple2dd *drive, bool online)
{
    drive->online = online;
}

/*
 * Write a byte to the disk in the drive. This is pretty similar to the
 * read function in that, once we do what we need with the segment, we
 * shift the drive position forward by one byte.
 */
void
apple2_dd_write(apple2dd *drive, vm_8bit byte)
{
    vm_segment_set(drive->data, apple2_dd_position(drive), byte);
    apple2_dd_shift(drive, 1);
}

/*
 * Set the write-protect status for the disk. Note that it was _disks_
 * that were write-protected in the past, sometimes by taping over a
 * chunk that was clipped out of the disk. So this function is somewhat
 * similar to just taping over or removing that tape.
 */
void
apple2_dd_write_protect(apple2dd *drive, bool protect)
{
    drive->write_protect = protect;
}
