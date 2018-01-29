/*
 * apple2.disk_drive.c
 */

#include "apple2.h"
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

    drive->locked = false;
    drive->track_pos = 0;
    drive->sector_pos = 0;
    drive->online = false;
    drive->write_protect = true;
    drive->mode = DD_READ;
    drive->phase_state = 0;
    drive->last_phase = 0;

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
 * Evaluate the value of the phase_state against the last known phase,
 * and decide from there whether we should step forward or backward by a
 * half-track. This function, as a side-effect, will update the last
 * phase to be the current phase state if the step was successful.
 */
void
apple2_dd_phaser(apple2dd *drive)
{
    int phase = drive->phase_state;
    int last = drive->last_phase;

    // This is a bit of trickery; there is no phase state for 0x10 or
    // 0x0, but we want to pretend like the phase is "next" to the bit
    // we're operating with for the purpose of establishing a direction.
    if (phase == 0x1 && last == 0x8) {
        phase = 0x10;
    } else if (phase == 0x8 && last == 0x1) {
        phase = 0x0;
    }

    // We only want to respond to adjacent phases, so if the last phase
    // shifted in _any_ direction is not equal to the phase state, then
    // we should do nothing.
    if (phase != (last << 1) ||
        phase != (last >> 1)
       ) {
        return;
    }

    // If phase > last, then we must move the head forward by a half
    // track. If it's < last, then we move the head backward, again by a
    // half track.
    if (phase > last) {
        apple2_dd_step(drive, 1);
    } else if (phase < last) {
        apple2_dd_step(drive, -1);
    }

    // Recall our trickery above with the phase variable? Because of it,
    // we have to save the phase_state field into last_phase, and not
    // the pseudo-value we assigned to phase.
    drive->last_phase = drive->phase_state;
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
    // When locked is true, we shouldn't shift our position by any
    // number.
    if (drive->locked) {
        return;
    }

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
apple2_dd_write(apple2dd *drive)
{
    vm_segment_set(drive->data, apple2_dd_position(drive), drive->latch);
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

/*
 * Half of all the disk drive switches deal with turning on and off the
 * different phases of the stepper. We handle all of them here.
 */
void
apple2_dd_switch_phase(apple2dd *drive, size_t addr)
{
    switch (addr & 0xF) {
        case 0x0: drive->phase_state &= ~0x1; break;
        case 0x1: drive->phase_state |=  0x1; break;
        case 0x2: drive->phase_state &= ~0x2; break;
        case 0x3: drive->phase_state |=  0x2; break;
        case 0x4: drive->phase_state &= ~0x4; break;
        case 0x5: drive->phase_state |=  0x4; break;
        case 0x6: drive->phase_state &= ~0x8; break;
        case 0x7: drive->phase_state |=  0x8; break;
    }
}

/*
 * This function handles all of the switch behavior that handles drive
 * metadata, like what drive is turned on, or which one is selected.
 */
void
apple2_dd_switch_drive(apple2 *mach, size_t addr)
{
    switch (addr) {
        case 0x8:
            apple2_dd_turn_on(mach->drive1, false);
            apple2_dd_turn_on(mach->drive2, false);
            break;

        case 0x9:
            apple2_dd_turn_on(mach->selected_drive, true);
            break;

        case 0xA:
            mach->selected_drive = mach->drive1;
            break;

        case 0xB:
            mach->selected_drive = mach->drive2;
            break;

        case 0xE:
            mach->selected_drive->mode = DD_READ;
            break;

        case 0xF:
            mach->selected_drive->mode = DD_WRITE;
            break;
    }
}

/*
 * If the disk drive is in write mode, we are allowed (!) to set the
 * latch value to whatever is passed in here.
 *
 * What's a latch value? Good question! It's basically a placeholder for
 * data to be committed (written) to disk. Writing via the Disk II drive
 * is a two-legged process; one, set the latch value; two, actually
 * write the latch value to the disk. As to why things are done this
 * way, I can only imagine that there were technical reasons for
 * essentially requiring the data to be written to be onboard the disk
 * drive itself.
 */
void
apple2_dd_switch_latch(apple2dd *drive, vm_8bit value)
{
    if (drive->mode == DD_WRITE) {
        drive->latch = value;
    }
}

/*
 * This function handles the logic for reading and/or writing to a Disk
 * II drive. What exactly happens here depends very much on the drive
 * mode, as well as whether or not we consider the disk to be
 * write-protected.
 */
vm_8bit
apple2_dd_switch_rw(apple2dd *drive)
{
    // If we are in read mode OR if we are working with a
    // write-protected disk, then all operations are interpreted as
    // read operations. If we are specifically in write mode, and in the
    // else condition we can say that the drive is not write-protected,
    // then we will write the latch data to the drive.
    if (drive->mode == DD_READ || drive->write_protect) {
        return apple2_dd_read(drive);
    } else if (drive->mode == DD_WRITE) {
        apple2_dd_write(drive);
    }

    return 0;
}

/*
 * This function handles reads to any of the disk II controller
 * addresses. Note that it's possible to write to a disk with a call to
 * this "read" function! Pay less attention to where these functions are
 * mapped, and pay more attention to the specific behavior of the
 * address switches being used.
 */
SEGMENT_READER(apple2_dd_switch_read)
{
    apple2 *mach = (apple2 *)_mach;
    apple2dd *drive = mach->selected_drive;

    // A nibble is a half-byte... not to be confused with the .NIB file
    // format
    size_t nib = addr & 0xF;

    // This might not be _right_... a better solution might be to bail
    // out unless the address indicates that the operation is not on a
    // specific drive, like turning drives off, or selecting a new
    // drive.
    if (drive == NULL) {
        drive = mach->drive1;
    }
    
    // In the first if block, we will handle 0x0..0x8; in the second if,
    // we'll do 0x9..0xB, 0xE, and 0xF.
    if (nib < 0x9) {
        apple2_dd_switch_phase(drive, nib);
    } else if (nib < 0xC || nib > 0xD) {
        apple2_dd_switch_drive(mach, nib);
    }

    // This is the read/write address... various states of the disk
    // drive will dictate what we do here.
    if (nib == 0xC) {
        return apple2_dd_switch_rw(drive);
    } else if (nib == 0xD) {
        // In a read context, accessing the latch switch will pass a
        // zero value into the latch. (The latch value will only be
        // committed if the drive itself is in write mode.)
        apple2_dd_switch_latch(drive, 0);
    }

    return 0;
}

/*
 * A decent portion of the logic in this function is similar to the
 * switch_read function, the defining difference being that here we have
 * a potentially-nonzero value to pass into the switch_latch function.
 * As such I have not commented much on the code here outside of what
 * can happen specifically in switch_write(); in the future it wouldn't
 * be a bad idea to refactor this common code into its own function.
 */
SEGMENT_WRITER(apple2_dd_switch_write)
{
    apple2 *mach = (apple2 *)_mach;
    apple2dd *drive = mach->selected_drive;
    size_t nib = addr & 0xF;

    if (drive == NULL) {
        drive = mach->drive1;
    }
    
    if (nib < 0x9) {
        apple2_dd_switch_phase(drive, nib);
    } else if (nib < 0xC || nib > 0xD) {
        apple2_dd_switch_drive(mach, nib);
    }

    // It's possible to attempt a "read" from a disk drive while doing a
    // write to the $C0nC address; all this does in effect is to shift
    // the disk forward. The more likely thing is that, if we are in
    // write mode, we will commit the latch value to disk; but that can
    // happen from either this particular function or from the
    // switch_read function.
    if (nib == 0xC) {
        apple2_dd_switch_rw(drive);
    } else if (nib == 0xD) {
        // The only way to get a latch value that is non-zero is to
        // write to the $C0nD address, where n is the address of one of
        // the disk controller ROMs. And even then, the drive needs to
        // be in write mode.
        apple2_dd_switch_latch(drive, value);
    }
}

/*
 * Map the Disk II drive switch addresses.
 */
void
apple2_dd_map(vm_segment *seg)
{
    size_t addr;

    for (addr = 0xC0E0; addr < 0xC0F0; addr++) {
        vm_segment_read_map(seg, addr, apple2_dd_switch_read);
        vm_segment_write_map(seg, addr, apple2_dd_switch_write);
    }

    for (addr = 0xC0F0; addr < 0xC100; addr++) {
        vm_segment_read_map(seg, addr, apple2_dd_switch_read);
        vm_segment_write_map(seg, addr, apple2_dd_switch_write);
    }
}
