/*
 * apple2.dec.c
 */

#include <stdbool.h>

#include "vm_segment.h"

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

int 
apple2_dec_sector(vm_segment *dest, vm_segment *src, int doff, int soff)
{
    vm_8bit conv[0x157];
    vm_8bit xor[0x156];
    vm_8bit lval;
    int i;

    int prologue = soff;
    int epilogue = soff + 9 + 0x157;

    // Let's validate that there's really a sector where we think
    // there's one.
    if (vm_segment_get(src, prologue) != 0xd5 ||
        vm_segment_get(src, prologue + 1) != 0xaa ||
        vm_segment_get(src, prologue + 2) != 0xad ||
        vm_segment_get(src, epilogue) != 0xde ||
        vm_segment_get(src, epilogue + 1) != 0xaa ||
        vm_segment_get(src, epilogue + 2) != 0xeb
       ) {
        return 0;
    }

    // Here we mean to convert the 6-and-2 encoded bytes back into its
    // first intermediate form
    for (i = 0; i < 0x157; i++) {
        conv[i] = conv6bit[vm_segment_get(src, soff + i + 9) & 0x7f];
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
