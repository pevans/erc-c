/*
 * apple2.dd.c
 *
 * This file encompasses the logic we use to handle disk drives in the
 * Apple II machine. You'll find a lot of references to what may seem to
 * be hardware implementation details, like stepper motor phases, but
 * these are necessary because those details were exposed to software,
 * and software was required to use those details to correctly position
 * itself.
 */

#include "apple2.dd.h"
#include "apple2.dec.h"
#include "apple2.enc.h"
#include "apple2.h"

// This is a small sort of state machine, which I adapted from AppleInPC
// (https://github.com/sosaria7/appleinpc) because I could not honestly
// think of a cleaner way of representing the step transitions that are
// possible. All credit to that project.
static int stepper_fsm[][8] =
{
	{  0,  0,  0,  0,  0,  0,  0,  0 },	// 0000
	{  0, -1, -2, -3,  0,  3,  2,  1 },	// 1000
	{  2,  1,  0, -1, -2, -3,  0,  3 },	// 0100
	{  1,  0, -1, -2, -3,  0,  3,  2 },	// 1100
	{  0,  3,  2,  1,  0, -1, -2, -3 },	// 0010
	{  0, -1,  0,  1,  0, -1,  0,  1 },	// 1010
	{  3,  2,  1,  0, -1, -2, -3,  0 },	// 0110
	{  2,  1,  0, -1, -2, -3,  0,  3 },	// 1110
	{ -2, -3,  0,  3,  2,  1,  0, -1 },	// 0001
	{ -1, -2, -3,  0,  3,  2,  1,  0 },	// 1001
	{  0,  1,  0, -1,  0,  1,  0, -1 },	// 0101
	{  0, -1, -2, -3,  0,  3,  2,  1 },	// 1101
	{ -3,  0,  3,  2,  1,  0, -1, -2 },	// 0011
	{ -2, -3,  0,  3,  2,  1,  0, -1 },	// 1011
	{  0,  3,  2,  1,  0, -1, -2, -3 },	// 0111
	{  0,  0,  0,  0,  0,  0,  0,  0 }	// 1111
};

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
    drive->image = NULL;

    drive->locked = false;
    drive->track_pos = 0;
    drive->sector_pos = 0;
    drive->online = false;
    drive->write_protect = true;
    drive->mode = DD_READ;
    drive->phase_state = 0;
    drive->last_phase = 0;
    drive->image_type = DD_NOTYPE;

    return drive;
}

/*
 * If we suppose that DOS 3.3 and ProDOS disk _images_ have their tracks
 * and sectors laid out in a logical or linear sense (tracks 0-35,
 * sectors 0-16, in that order), then this function will return the
 * sector within one track as it would be _physically_ laid out on the
 * disk media.
 *
 * As you might suppose, the physical layout is different from the
 * logical one. Sectors are physically laid out in an interleaved
 * manner, where a sector from the first half of a track are mixed in
 * with sectors from the latter half; this is keep the distance your
 * disk drive would need to rotate to get to an arbitrary, given sector
 * at a somewhat minimum. (For instance: suppose you were on sector 1,
 * and you needed sector 2, but the disk media had already rotated to
 * sector 3; you now need to do another full rotation to get back to
 * sector 2.)
 */
int
apple2_dd_sector_num(int type, int sect)
{
    static const int dos33[] = {
        0x0, 0x7, 0xe, 0x6, 0xd, 0x5, 0xc, 0x4,
        0xb, 0x3, 0xa, 0x2, 0x9, 0x1, 0x8, 0xf,
    };

    static const int prodos[] = {
        0x0, 0x8, 0x1, 0x9, 0x2, 0xa, 0x3, 0xb,
        0x4, 0xc, 0x5, 0xd, 0x6, 0xe, 0x7, 0xf,
    };

    const int *sectab;

    // Booooo
    if (sect < 0 || sect > 15) {
        return 0;
    }

    switch (type) {
        case DD_DOS33:
            sectab = dos33;
            break;
            
        case DD_PRODOS:
            sectab = prodos;
            break;

        // We also allow default behavior to mimic NIBBLE, but there's
        // probably no "right" default behavior to use.
        default:
        case DD_NIBBLE:
            return sect;
    }

    return sectab[sect];
}

/*
 * Insert a "disk" into the drive, such that a disk is delivered to us
 * through a FILE stream. Return an error code if the disk format is
 * something we cannot accept.
 */
int
apple2_dd_insert(apple2dd *drive, FILE *stream, int type)
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

    drive->image = vm_segment_create(finfo.st_size);
    drive->track_pos = 0;
    drive->sector_pos = 0;

    // Read the data from the stream and write into the memory segment
    err = vm_segment_fread(drive->image, stream, 0, finfo.st_size);
    if (err != OK) {
        log_critical("Could not read data into disk drive");
        return err;
    }

    drive->stream = stream;
    drive->image_type = type;

    // Now we need to build the data segment
    apple2_dd_encode(drive);

    return OK;
}

/*
 * Encode the drive image segment into the drive data segment using
 * 6-and-2 encoding, if necessary. (It is not necessary if the
 * image_type is DD_NIBBLE.)
 */
int
apple2_dd_encode(apple2dd *drive)
{
    switch (drive->image_type) {
        case DD_NIBBLE:
            drive->data = apple2_enc_nib(drive->image);
            break;

        case DD_DOS33:
        case DD_PRODOS:
            drive->data = apple2_enc_dos(drive->image_type, drive->image);
            break;

        default:
            log_critical("Unknown image type");
            return ERR_INVALID;
    }

    return OK;
}

/*
 * Save the contents of the drive back to the file system (given as the
 * stream field in the drive struct).
 */
void
apple2_dd_save(apple2dd *drive)
{
    // First bring the image segment back into sync with with the data
    // segment.
    apple2_dd_decode(drive);

    if (drive->stream) {
        rewind(drive->stream);
        vm_segment_fwrite(drive->image, drive->stream, 0, drive->image->size);
    }
}

/*
 * Decode the drive data segment into the drive image segment, reversing
 * the 6-and-2 encoding (if need be -- see note on DD_NIBBLE for the
 * encode function).
 */
int
apple2_dd_decode(apple2dd *drive)
{
    switch (drive->image_type) {
        case DD_NIBBLE:
            apple2_dec_nib(drive->image, drive->data);
            break;

        case DD_DOS33:
        case DD_PRODOS:
            apple2_dec_dos(drive->image_type, drive->image, drive->data);
            break;

        default:
            log_critical("Unknown image type");
            return ERR_INVALID;
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

    if (!drive->online) {
        return;
    }

    // Look up the number of steps to move according to the current
    // phase and the current track position.
    apple2_dd_step(drive, 
                   stepper_fsm[phase][drive->track_pos & 0x7]);

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

    int track_offset = (drive->track_pos / 2) * ENC_ETRACK;
    return track_offset + drive->sector_pos;
}

/*
 * Read a single byte from the disk drive, at its current position, and
 * then shift the head by 1 byte.
 */
vm_8bit
apple2_dd_read(apple2dd *drive)
{
    // If we have no disk in the drive, we will always return a zero
    // byte
    if (drive->data == NULL) {
        return 0;
    }

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
        // Save off anything else we have left
        apple2_dd_save(drive);

        vm_segment_free(drive->data);
        vm_segment_free(drive->image);
        drive->data = NULL;
        drive->image = NULL;
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

    if (drive->sector_pos >= ENC_ETRACK) {
        // We need to reset the sector pos to zero, because...
        drive->sector_pos = 0;
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

    // The sector position is rehomed to zero whenever we step a
    // non-zero length.
    if (steps) {
        drive->sector_pos = 0;
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
    // If there's no disk in the drive, then we can't write anything
    if (drive->data == NULL) {
        return;
    }

	if (drive->latch & 0x80) {
		vm_segment_set(drive->data, apple2_dd_position(drive), drive->latch);
		apple2_dd_shift(drive, 1);
	}
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

    apple2_dd_phaser(drive);
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
            if (mach->selected_drive) {
                apple2_dd_turn_on(mach->selected_drive, true);
            }
            break;

        case 0xA:
            mach->selected_drive = mach->drive1;
            break;

        case 0xB:
            mach->selected_drive = mach->drive2;
            break;

        case 0xE:
            if (mach->selected_drive) {
                mach->selected_drive->mode = DD_READ;
            }
            break;

        case 0xF:
            if (mach->selected_drive) {
                mach->selected_drive->mode = DD_WRITE;
            }
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

    for (addr = 0xC0E0; addr < 0xC100; addr++) {
        vm_segment_read_map(seg, addr, apple2_dd_switch_read);
        vm_segment_write_map(seg, addr, apple2_dd_switch_write);
    }
}
