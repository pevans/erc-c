#ifndef _APPLE2_DISK_DRIVE_H
#define _APPLE2_DISK_DRIVE_H

/*
 * Forward declaration of apple2dd for some files (e.g. apple2.h) which
 * want to know about us before we have actually defined the struct.
 */
struct apple2dd;
typedef struct apple2dd apple2dd;

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#include "apple2.h"
#include "vm_bits.h"
#include "vm_segment.h"

/*
 * Define the kind of image types that our disk media currently has. The
 * image type is something which is given to us when the disk is
 * "inserted", and it can't be changed during machine operation.
 * Well--it could--but we don't have a good reason to do so.
 */
enum apple2_dd_type {
    DD_NOTYPE,
    DD_DOS33,
    DD_PRODOS,
    DD_NIBBLE,
};

/*
 * These are the possible modes a drive can be in.
 */
enum apple2_dd_mode {
    DD_READ,
    DD_WRITE,
};

/*
 * This is the length of a typical disk that is formatted in either DOS
 * 3.3 or ProDOS. The NIB definition is the size of one such image when
 * it is nibbilized with 6-and-2 encoding.
 */
#define _140K_ 143360
#define _140K_NIB_ 233008

/*
 * And this is the length of a disk that has been formatted as a nibble
 * file (*.NIB). This is not an Apple thing, exactly; it's more of an
 * emulator thing, that emulators had used to try and get around copy
 * protection in emulation. It does complicate disk drive operation!
 */
#define _240K_ 245760

#define MAX_DRIVE_STEPS 140

/*
 * This is the last _accessible_ sector position within a track (you can
 * have 0 - 4095).
 */
#define MAX_SECTOR_POS 4095

/*
 * These are the possible phases that can energize the cogs in a disk
 * drive motor. We represent them as bits, as more than one phase can be
 * on at the same time.
 */
#define DD_PHASE1 0x1
#define DD_PHASE2 0x2
#define DD_PHASE3 0x4
#define DD_PHASE4 0x8

struct apple2dd {
    /*
     * Inside the disk drive there is a stepper motor, and it's
     * controlled through four "phases", which are a bit hard to
     * describe. Imagine four points on a wheel; suppose in order to
     * to roll the wheel forward on the ground, you could only do so by
     * controlling which point is facing the ground. A smooth rotation
     * would always require you choose the point which is
     * counter-clockwise from the ground at a 90º angle; and the point
     * which was on the ground, is now clockwise from the ground, at
     * 90º. Going backwards is similar, except you choose the point
     * clockwise from the ground; and the point on the ground now goes
     * counter-clockwise.
     *
     * To advance the motor, you need to turn on an adjacent phase; if
     * phase 1 is on, turn on phase 2 and turn off phase 1; this allows
     * you to go "forward"; and then turn on phase 3, and turn off phase
     * 2; and you wrap around, so you turn on phase 0 and turn off phase
     * 3. And vice versa for going "backward". In this field, then, we
     * really care about only four bits; 0x1, 0x2, 0x4, and 0x8; and, in
     * particular, we care about the adjacent relations of those high
     * and low bits.
     *
     * It's really like if you unrolled the surface of the wheel and
     * laid it out as a flat line, but still with those points defined.
     * Does that make sense?
     */
    vm_8bit phase_state;
    vm_8bit last_phase;
    
    /*
     * Data is written via a "latch", and happens in two steps; one, you
     * set the latch; two, you commit the write. By steps, I mean two
     * separate instructions--not necessarily adjacent to each other,
     * but in some sequence and in that order.
     */
    vm_8bit latch;

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
     * This is a weirder one, because while DOS cares about sectors, we
     * don't really. We just need to know how to find the right position
     * to work with in the disk image.
     *
     * Each track has 16 sectors, and each sector has 256 bytes. We can
     * then say that each track is 4k (4,096) bytes large. So while our
     * track_pos can tell us which 4k chunk we're in, the sector_pos has
     * to tell us where we are _within_ the track. Again -- we don't
     * care about the sector number, really. So the sector_pos field is
     * tracking the byte offset from the beginning of the track, such
     * that 0 ≤ sector_pos < 4096.
     */
    int sector_pos;

    /*
     * The data segment holds the literal data that the software running
     * in an Apple II would expect to see when accessing the device,
     * which is to say, data that is 6-and-2 encoded. The image segment
     * holds the data we literally read from a disk image file, which
     * (in most cases) is not 6-and-2 encoded. We also define an image
     * type (see apple2_dd_type for enums).
     */
    vm_segment *data;
    vm_segment *image;
    int image_type;

    /*
     * This is the means by which we can save the image data back to the
     * origin stream, if possible.
     */
    FILE *stream;

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

    /*
     * This is not a technical part of the Disk II drive. When this
     * field is true, we will not shift the head after a read or a
     * write; when false, we will. This is mainly useful for code like
     * the disassembler which just wants to look at data and not apply
     * other side-effects.
     */
    bool locked;
};

extern SEGMENT_READER(apple2_dd_switch_read);
extern SEGMENT_WRITER(apple2_dd_switch_write);
extern apple2dd *apple2_dd_create();
extern int apple2_dd_decode(apple2dd *);
extern int apple2_dd_encode(apple2dd *);
extern int apple2_dd_insert(apple2dd *, FILE *, int);
extern int apple2_dd_position(apple2dd *);
extern int apple2_dd_sector_num(int, int);
extern vm_8bit apple2_dd_read(apple2dd *);
extern vm_8bit apple2_dd_switch_rw(apple2dd *);
extern void apple2_dd_eject(apple2dd *);
extern void apple2_dd_free(apple2dd *);
extern void apple2_dd_map(vm_segment *);
extern void apple2_dd_phaser(apple2dd *);
extern void apple2_dd_save(apple2dd *);
extern void apple2_dd_set_mode(apple2dd *, int);
extern void apple2_dd_shift(apple2dd *, int);
extern void apple2_dd_step(apple2dd *, int);
extern void apple2_dd_switch_drive(apple2 *, size_t);
extern void apple2_dd_switch_latch(apple2dd *, vm_8bit);
extern void apple2_dd_switch_phase(apple2dd *, size_t);
extern void apple2_dd_turn_on(apple2dd *, bool);
extern void apple2_dd_write(apple2dd *);
extern void apple2_dd_write_protect(apple2dd *, bool);

#endif
