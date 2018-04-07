/*
 * apple2.dec.c
 *
 * Decode 6-and-2 encoding to get back to the "raw" state that image
 * data has. You can read more on why this is necessary in apple2.enc.c.
 */

#include <stdbool.h>

#include "apple2/dd.h"
#include "apple2/dec.h"
#include "apple2/enc.h"
#include "vm_segment.h"

/*
 * This table are what we convert from the 6-and-2 encoded form back
 * into an intermediate form of data that has been XOR'd with each other
 * byte. If that makes any sense.
 */
static vm_8bit conv6bit[] = {
//  00    01    02    03    04    05    06    07    08    09    0a    0b    0c    0d    0e    0f 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 00
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x04, 0xff, 0xff, 0x08, 0x0c, 0xff, 0x10, 0x14, 0x18, // 10
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x20, 0xff, 0xff, 0xff, 0x24, 0x28, 0x2c, 0x30, 0x34, // 20
    0xff, 0xff, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0xff, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, // 30
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6c, 0xff, 0x70, 0x74, 0x78, // 40
    0xff, 0xff, 0xff, 0x7c, 0xff, 0xff, 0x80, 0x84, 0xff, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, // 50
    0xff, 0xff, 0xff, 0xff, 0xff, 0xa4, 0xa8, 0xac, 0xff, 0xb0, 0xb4, 0xb8, 0xbc, 0xc0, 0xc4, 0xc8, // 60
    0xff, 0xff, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xff, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc, // 70
};

/*
 * Decode an entire DOS 3.3-ordered disk, and copy the contents into the
 * given segment.
 */
int
apple2_dec_dos(int sectype, vm_segment *dest, vm_segment *src)
{
    int i, doff, tracklen;

    if (dest == NULL || src == NULL) {
        return OK;
    }

    for (i = 0, doff = 0; i < ENC_NUM_TRACKS; i++) {
        tracklen = apple2_dec_track(sectype, dest, src, doff, i);

        // Something went wrong...
        if (tracklen != ENC_DTRACK) {
            return ERR_BADFILE;
        }

        doff += tracklen;
    }

    return OK;
}

/*
 * NIB files are literally 6-and-2 encoded to begin with, so there's not
 * anything we need to do to decode them (other than copy the data into
 * the destination segment).
 */
int
apple2_dec_nib(vm_segment *dest, vm_segment *src)
{
    // It's "ok" if you pass in NULL params; the only time you ever
    // would is in testing, because we presume you are testing some
    // other aspect of the code there. (Good example: when your test is
    // not actually testing apple2_dec_nib, but something that calls
    // it.)
    if (dest == NULL || src == NULL) {
        return OK;
    }

    return vm_segment_copy(dest, src, 0, 0, src->size);
}

/*
 * Decode a 6-and-2 encoded track, and write the decoded data into dest.
 * This should return ENC_DTRACK bytes; if not, something went wrong.
 */
int
apple2_dec_track(int sectype, vm_segment *dest, vm_segment *src, int doff, int track)
{
    int orig = doff;
    int sect, sectlen, soff;

    soff = (track * ENC_ETRACK) + ENC_ETRACK_HEADER;

    for (sect = 0; sect < ENC_NUM_SECTORS; sect++) {
        doff =
            (track * ENC_DTRACK) +
            (apple2_dd_sector_num(sectype, sect) * ENC_DSECTOR);

        // This is going to be 256, for all intents and purposes
        sectlen = apple2_dec_sector(dest, src, doff, soff + ENC_ESECTOR_HEADER);

        // If _not_, then that reflects a kind of error condition. Let's
        // bail.
        if (sectlen != ENC_DSECTOR) {
            return 0;
        }

        soff += ENC_ESECTOR;
    }

    return ENC_DTRACK;
}

/*
 * This function may be difficult to follow, but let me outline what
 * it's trying to do:
 *
 * 1. We convert the data in the src segment from the soff offset using
 * the conv6bit lookup table into an intermediate form held in the conv
 * buffer;
 *
 * 2. Which is then XOR'd with each previous byte, and finally with a
 * checksum byte that is at the end of the conv buffer, and storing the
 * result of that into the xor buffer;
 *
 * 3. Which we then loop on to recombine the 6-bit bytes at 0x56..0x156
 * with the least significant bits that are held in the bytes from
 * 0x00..0x56.
 *
 * 4. The result of which is written to the dest segment, using the doff
 * offset.
 *
 * A lot of this complexity comes from technical restrictions on the
 * floppy disk media that were used at the time--namely that there could
 * be no more than a certain number of zero bits in a row.
 */
int 
apple2_dec_sector(vm_segment *dest, vm_segment *src, int doff, int soff)
{
    /*
     * This is a buffer that holds the data that we converted back from
     * the 6-and-2 encoded form.
     */
    vm_8bit conv[0x157];

    /*
     * This is another buffer, holding the data that we XOR'd to bring
     * it back to the form it had before it had been XOR'd in the encode
     * process.
     */
    vm_8bit xor[0x156];

    /*
     * The last byte that we XOR'd (see the xor loop below).
     */
    vm_8bit lval;

    int i;

    /*
     * The header_ok variable is true if the beginning byte markers are
     * there.
     */
    int header = soff;
    bool header_ok =
        vm_segment_get(src, header) == 0xd5 &&
        vm_segment_get(src, header + 1) == 0xaa &&
        vm_segment_get(src, header + 2) == 0xad;

    // The footer_ok variable will be true if the ending byte markers we
    // expect to see are actually there.
    int footer = soff + 3 + 0x157;
    bool footer_ok =
        vm_segment_get(src, footer) == 0xde &&
        vm_segment_get(src, footer + 1) == 0xaa &&
        vm_segment_get(src, footer + 2) == 0xeb;

    // Let's validate that there's really a sector where we think
    // there's one.
    if (!header_ok || !footer_ok) {
        return 0;
    }

    // Here we mean to convert the 6-and-2 encoded bytes back into its
    // first intermediate form
    for (i = 0; i < 0x157; i++) {
        conv[i] = conv6bit[vm_segment_get(src, soff + i + 3) & 0x7f];
    }

    // Originally, we XOR'd each byte when encoding; so we need to do
    // another XOR, in pretty much the same manner.
    for (i = 0, lval = 0; i < 0x156; i++) {
        xor[i] = lval ^ conv[i];
        lval = xor[i];
    }

    // Now we need to copy every byte back into its form that would be
    // found on the original disk image. We're using the same sort of
    // loop that jumps around three different sections of the array per
    // iteration.
    for (i = 0; i < 0x56; i++) {
        vm_8bit offac, off56;
        vm_8bit vac, v56, v00;

        offac = i + 0xac;
        off56 = i + 0x56;

        // Recall that the least significant bits are packed into the
        // first 86 (0x56) bytes of the 6-and-2 scheme block. So what
        // we're doing here is grabbing the 6 _most_ significant bits
        // (which is offac + 0x56), then using an OR to pack on the
        // least significant bits from the those first 86 bytes.
        vac =
            (xor[offac + 0x56] & 0xfc) |
            ((xor[i] & 0x80) >> 7) |
            ((xor[i] & 0x40) >> 5);

        v56 =
            (xor[off56 + 0x56] & 0xfc) |
            ((xor[i] & 0x20) >> 5) |
            ((xor[i] & 0x10) >> 3);

        v00 =
            (xor[i + 0x56] & 0xfc) |
            ((xor[i] & 0x08) >> 3) |
            ((xor[i] & 0x04) >> 1);

        // If we wrap around to 00 or 01, as will likely do with offac,
        // don't do the set (it gets set with doff+i and v00).
        if (offac >= 0xac) {
            vm_segment_set(dest, doff + offac, vac);
        }

        // Set the rest!
        vm_segment_set(dest, doff + off56, v56);
        vm_segment_set(dest, doff + i, v00);
    }

    // Finally, we always return 256 since that's all we will be able to
    // write from the given block (the validation of which is done by
    // checking prologue/epilogue bytes first in this function).
    return 256;
}
