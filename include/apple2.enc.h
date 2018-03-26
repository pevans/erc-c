#ifndef _APPLE2_ENC_H_
#define _APPLE2_ENC_H_

#include "vm_bits.h"
#include "vm_segment.h"

/*
 * This is a magic number for the disk volume; I haven't experimented
 * with using other values for it, and this one in particular is used in
 * WinApple, so I went with it.
 */
#define ENC_VOLUME 0xfe

/*
 * The number of tracks that a DOS 3.3 disk would have.
 */
#define ENC_NUM_TRACKS 35

/*
 * The number of sectors within a single track of data for a DOS 3.3
 * disk.
 */
#define ENC_NUM_SECTORS 16

/*
 * A decoded sector is 256 bytes long
 */
#define ENC_DSECTOR 0x100

/*
 * And a track--being composed of 16 sectors--is 4096 bytes long, or 4k
 * bytes. (256 * 16 = 4096)
 */
#define ENC_DTRACK 0x1000

/*
 * An encoded sector is 396 bytes long, and is comprised of a sector
 * header plus padding bytes both before _and_ after the data field.
 */
#define ENC_ESECTOR 0x1a0

/*
 * An encoded track contains 16 sectors, as mentioned for ENC_DTRACK.
 * But it also contains some additional padding (48 bytes-worth).
 */
#define ENC_ETRACK 0x1a00

/*
 * A sector header consists of some byte markers--all byte markers in
 * 6-and-2 encoding are 3 bytes long--and also some metadata, such as
 * the track number, the sector number, the volume, and an XOR'd
 * combination of all three.
 */
#define ENC_ESECTOR_HEADER 0x13

/*
 * The track header (as mentioned for ENC_ETRACK) is 48 bytes of--well,
 * nothing really, just padding.
 */
#define ENC_ETRACK_HEADER 0x30

extern int apple2_enc_4n4(vm_segment *, int, vm_8bit); 
extern int apple2_enc_sector(vm_segment *, vm_segment *, int, int);
extern int apple2_enc_sector_header(vm_segment *, int, int, int);
extern int apple2_enc_track(int, vm_segment *, vm_segment *, int, int);
extern vm_segment *apple2_enc_dos(int, vm_segment *);
extern vm_segment *apple2_enc_nib(vm_segment *);

#endif
