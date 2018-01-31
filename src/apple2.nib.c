/*
 * apple2.nib.c
 */

#include "vm_segment.h"

static vm_8bit gcr62[] = {
//  00    01    02    03    04    05    06    07    08    09    0a    0b    0c    0d    0e    0f 
    0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6, 0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
    0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
    0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

vm_segment *
apple2_nib_decode(vm_segment *src)
{
    vm_segment *dest;

    // FIXME: we need to find the actual size of a nibble-ized DO/PO
    // order file
    dest = vm_segment_create(500000);

    return dest;
}

void
apple2_nib_decode_track(vm_segment *dest, vm_segment *src, int track)
{
}

/*
 * Encode the src segment of image data (e.g. from a disk) with 6-and-2
 * encoding; this will copy one 256 byte block from src into a 343-byte
 * block into dest. We work the given destination offset and source
 * offset, but care must be taken to ensure that dest contains enough
 * room to hold the values from src.
 */
void
apple2_nib_encode_sector(vm_segment *dest, vm_segment *src,
                         int doff, int soff)
{
    int i, di;
    vm_8bit lastval, curval;
    
    // The init array contains the src segment's 256 bytes converted
    // into 342 bytes, but more works needs to be done to get it into
    // proper 6-and-2 encoding. The xor array will contain the XOR'd
    // version of init, but with an extra value tagged in as a checksum.
    vm_8bit init[0x156], xor[0x157];

    // This loop is really complicated; I'll annotate it as best I can.
    // To begin with, we mean to write the first 86 bytes for the
    // initial array.
    for (i = 0; i < 0x56; i++) {
        vm_8bit v = 0, vac, v56, v00;

        // We do the write by working with the src segment in rough
        // thirds. vac is the value offset by 0xAC, which is 0x56 * 2.
        // v56 is offset by 0x56, and v00 has no offset. In decimal
        // terms, vac is 172 bytes offset from 0, and v56 is 86 bytes
        // offset from 0.
        vac = vm_segment_get(src, soff+i+0xAC);
        v56 = vm_segment_get(src, soff+i+0x56);
        v00 = vm_segment_get(src, soff+i);

        // The value we ultimately want to write into the dest segment
        // is then mangled a bit. v begins life as zero, of course; it's
        // then OR'd with vac's first and second bits, but in reverse
        // order as it were; that is, bit 0 is promoted to bit 1 in v,
        // and bit 1 in vac is demoted to bit 0 in v. This is repeated
        // twice more, with v56 and v00. We now have filled in six bits.
        v = (v << 2) | ((vac & 0x1) << 1) | ((vac & 0x2) >> 1);
        v = (v << 2) | ((v56 & 0x1) << 1) | ((v56 & 0x2) >> 1);
        v = (v << 2) | ((v00 & 0x1) << 1) | ((v00 & 0x2) >> 1);

        // We then write this into the dest segment, shifted twice more,
        // so that all the bits that may be high will begin at the
        // "left" side, from bit 7 - bit 2, leaving bit 1 and 0 at zero.
        init[i] = v << 2;
    }

    // The last two bytes written must be AND'd so that only the first
    // six bits can be high. But because the bit 1 and 0 are already
    // low, this has the effect of limiting the high bits to bits 5-2.
    init[i-2] &= 0x3F;
    init[i-1] &= 0x3F;

    // The rest of the bytes may be copied from the src buffer into dest
    // without modification. (Phew!)
    for (i = 0x00, di = 0x56; i < 0x100; i++, di++) {
        init[di] = vm_segment_get(src, soff+i);
    }

    // Here we will XOR each byte with each successive byte, and store
    // that into the xor array.
    for (i = 0, lastval = 0; i < 0x156; i++) {
        curval = init[i];
        xor[i] = curval ^ lastval;
        lastval = curval;
    }

    // But we need one more byte in the xor array; this is just the last
    // value from init.
    xor[i] = lastval;

    // Now we use the gcr table for 6-and-2 encoding to take the XOR'd
    // values and represent them as they should be in the destination
    // segment.
    for (i = 0; i < 0x157; i++) {
        vm_segment_set(dest, doff+i, gcr62[xor[i] >> 2]);
    }
}
