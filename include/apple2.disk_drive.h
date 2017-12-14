#ifndef _APPLE2_DISK_DRIVE_H
#define _APPLE2_DISK_DRIVE_H

/*
 * These are the possible modes a drive can be in.
 */
enum apple2_disk_drive_mode {
    DD_READ,
    DD_WRITE,
};

#define MAX_DRIVE_STEPS 70

typedef struct {
    /*
     * Disk II drives allow the stepper to move in half-tracks, so we
     * track (pun intended) the position of the head in those
     * half-tracks rather than in full tracks.
     *
     * For example, if track_pos is 4, then the effective track is 2. If
     * the track_pos is 9, then the effective track is 4, except that
     * the head is on the half-track position.
     *
     * There are, at most, 35 tracks in a conventional disk, so there
     * would be at most 70 track positions that we can iterate to.
     */
    int track_pos;

    /*
     * The data field is where the actual byte data for the image is
     * kept.
     */
    vm_segment *data;

    /*
     * A disk drive may be "off" or "on", regardless of whether it's
     * been selected by the peripheral interface.
     */
    bool online;

    /*
     * This is one of DD_READ or DD_WRITE (defined in the enum above).
     * The drive can only read or write at once, and the mode of
     * operation must be made explicit through this mechanism.
     */
    int mode;

    /*
     * Write protection is an attribute of the disk. Back in the day, a
     * disk would have a small segment cut out of the disk on the side;
     * this would make it writeable. A disk without that would be
     * write-protected. You could take a writeable disk and make it
     * write-protected simply by putting some solid-colored tape over
     * the cut-out.
     *
     * For our purposes, write protection is a simply boolean attribute
     * that you can enable or disable on the drive.
     */
    bool write_protect;
} apple2_disk_drive;

extern apple2_disk_drive *apple2_disk_drive_create();
extern void apple2_disk_drive_free(apple2_disk_drive *);
extern void apple2_disk_drive_step(apple2_disk_drive *, int);
extern void apple2_disk_drive_set_mode(apple2_disk_drive *, int);
extern void apple2_disk_drive_turn_on(apple2_disk_drive *, bool);
extern void apple2_disk_drive_write_protect(apple2_disk_drive *, bool);

#endif
