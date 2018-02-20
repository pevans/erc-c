#include <criterion/criterion.h>

#include "apple2.dd.h"
#include "apple2.enc.h"
#include "vm_segment.h"

/*
 * This is the DOS 3.3 order; just to have something to use as a basis
 */
static int sectab[] = {
    0x0, 0x7, 0xe, 0x6, 0xd, 0x5, 0xc, 0x4,
    0xb, 0x3, 0xa, 0x2, 0x9, 0x1, 0x8, 0xf,
};

/*
 * A sector fixture; something we can use when testing sector encoding
 * and decoding. This is the unencoded form of a sector, in 256 bytes.
 */
static vm_8bit f_sector[] = {
    0xdd, 0xab, 0xce, 0xd9, 0xc3, 0xf6, 0xdd, 0x77, 0x0e, 0x60, 0x6d, 0x95, 0xbd, 0xe3, 0x74, 0xc0,
    0xd4, 0xa7, 0x3f, 0xc2, 0x39, 0x37, 0xfa, 0xa7, 0xd1, 0xa9, 0xc9, 0x5e, 0xd3, 0x28, 0x47, 0x04,
    0x4f, 0xec, 0xfc, 0xc1, 0x5e, 0x90, 0x9a, 0x7d, 0x72, 0x36, 0xd0, 0x23, 0x90, 0xe5, 0x18, 0xeb,
    0x8a, 0x80, 0xad, 0xda, 0x85, 0xe4, 0xe3, 0x34, 0x72, 0x5c, 0x51, 0x18, 0xf0, 0xec, 0x3f, 0xcf,
    0xf7, 0xc1, 0x61, 0xe7, 0xad, 0x78, 0xea, 0x8a, 0xa1, 0x53, 0xbc, 0x05, 0x06, 0x77, 0x40, 0xb1,
    0xfb, 0x21, 0xf4, 0xec, 0x7a, 0xd3, 0xee, 0xcd, 0x99, 0xfa, 0x95, 0x39, 0xf8, 0xe8, 0x2d, 0xf5,
    0x40, 0x1a, 0x68, 0x0b, 0x7e, 0x96, 0x4c, 0x05, 0x03, 0x74, 0xa9, 0xca, 0x1d, 0x30, 0xf0, 0x96,
    0x7d, 0xe9, 0x24, 0xca, 0x6b, 0x09, 0x50, 0x69, 0x35, 0xe0, 0x1b, 0xd8, 0xfb, 0x62, 0x47, 0xe6,
    0x6a, 0xad, 0x3a, 0xd2, 0x5c, 0xe6, 0xda, 0x5f, 0x62, 0xfb, 0x02, 0x61, 0x2b, 0x42, 0x7a, 0x44,
    0xa2, 0x2f, 0x97, 0xe5, 0x08, 0xc7, 0xf6, 0xe1, 0x7a, 0xd6, 0x6a, 0xc0, 0x11, 0xd6, 0xa4, 0x26,
    0xac, 0xe4, 0xfb, 0xf3, 0x28, 0x8a, 0x3b, 0x1b, 0x4c, 0x9c, 0x84, 0x55, 0x73, 0x4b, 0x4f, 0xc1,
    0x0a, 0xd8, 0x9e, 0xdc, 0x1b, 0x01, 0xec, 0x12, 0x41, 0x84, 0x15, 0x8c, 0xb9, 0x29, 0x5f, 0x34,
    0x99, 0x3e, 0x3b, 0xa8, 0xcf, 0x7c, 0x65, 0x6f, 0x60, 0x45, 0x73, 0xe4, 0xea, 0x84, 0x84, 0x8b,
    0xaa, 0x46, 0x72, 0xbc, 0xd2, 0x65, 0xbe, 0x5b, 0x2d, 0x24, 0xd6, 0x41, 0x88, 0xd0, 0x6a, 0x80,
    0x1e, 0x22, 0x7a, 0x44, 0x70, 0x17, 0x44, 0x18, 0x1f, 0x31, 0x41, 0xf5, 0xa0, 0xae, 0x1b, 0xe1,
    0x4f, 0x9f, 0xdb, 0x5a, 0x74, 0x90, 0x55, 0xe1, 0x49, 0xbf, 0x2d, 0x20, 0xd6, 0x03, 0x03, 0xa2,
};

/*
 * This is the 6-and-2 encoded form of the f_sector array above. It does
 * _not_ include the sector header, which is tested in a separate
 * function.
 */
static vm_8bit f_enc_sector[] = {
    0xd5, 0xaa, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf5, 0xaf, 0x9a, 0xd3, 0xfd, 0xb6, 0xcb,
    0xb5, 0xfa, 0xb5, 0xe6, 0xb9, 0xf3, 0xea, 0xe7, 0xd6, 0xdb, 0xad, 0xb9, 0xfe, 0xe7, 0xfd, 0xeb,
    0xfa, 0xee, 0xf3, 0xe9, 0xb7, 0xfa, 0xde, 0xcb, 0xf6, 0xcb, 0xcb, 0x96, 0xfa, 0xec, 0xaf, 0xaf,
    0xcb, 0xcb, 0xf7, 0xee, 0xec, 0xd3, 0xd9, 0xbb, 0xf6, 0xd9, 0xab, 0xbf, 0xcb, 0xcb, 0xb2, 0xa6,
    0xcb, 0x97, 0xf4, 0xf5, 0xac, 0xed, 0xb9, 0xad, 0xae, 0xdf, 0xce, 0xe9, 0xb5, 0xb5, 0x9f, 0xab,
    0xe9, 0xbc, 0x9e, 0xde, 0x9a, 0xb3, 0xb6, 0xd3, 0xdd, 0xbe, 0xd7, 0xb2, 0xd6, 0xb5, 0xac, 0xfc,
    0xce, 0xbe, 0x9e, 0x9f, 0xaf, 0xac, 0xe6, 0xcf, 0xcb, 0x9b, 0xfe, 0xac, 0xbc, 0xdc, 0xea, 0x9e,
    0xcd, 0xdd, 0xff, 0xfe, 0x9b, 0xf2, 0xbc, 0xce, 0xcf, 0xbd, 0xdc, 0xda, 0xfe, 0xcb, 0xb4, 0xb6,
    0xdf, 0x9d, 0xb3, 0xde, 0xf2, 0x9a, 0xf9, 0x9b, 0xb5, 0xf9, 0xfc, 0xe9, 0xce, 0xff, 0xfc, 0xbd,
    0x9a, 0xad, 0xce, 0xbc, 0xbd, 0x97, 0xf4, 0xb5, 0xad, 0x9b, 0xb6, 0xfa, 0xa6, 0xf3, 0xfc, 0xb2,
    0xaf, 0xdf, 0xd7, 0xb6, 0xf4, 0xdb, 0xbd, 0xac, 0xfc, 0xfb, 0xeb, 0x96, 0xcd, 0xaf, 0xfc, 0xb6,
    0xf5, 0xf4, 0x9f, 0xdc, 0xe6, 0xb3, 0xa7, 0xba, 0xbd, 0xcb, 0xe7, 0xed, 0x9d, 0xee, 0xf5, 0xea,
    0xbb, 0xcd, 0xbd, 0xce, 0xfa, 0xf5, 0xb6, 0x97, 0xce, 0xf6, 0xbd, 0xf4, 0xad, 0xed, 0xbe, 0xfa,
    0xdc, 0xf2, 0xfb, 0xdf, 0xbd, 0xbb, 0xb2, 0xbc, 0xf4, 0xfe, 0xed, 0xa7, 0xdd, 0xab, 0xdf, 0xda,
    0xee, 0xdc, 0xfa, 0xda, 0xeb, 0xb3, 0xd7, 0xb3, 0xdd, 0xfe, 0xbd, 0xb6, 0xbf, 0xb2, 0xb3, 0xf9,
    0xda, 0xeb, 0xcd, 0xfb, 0xf2, 0xae, 0x9e, 0xdd, 0xe7, 0xec, 0xe6, 0xf3, 0xee, 0xcd, 0xd6, 0xd9,
    0xb6, 0xa6, 0x9a, 0xf5, 0xdf, 0xe9, 0xa7, 0xba, 0xf3, 0x9f, 0xf3, 0xab, 0xb2, 0x97, 0xda, 0xef,
    0xf3, 0xb5, 0xb4, 0xee, 0x9f, 0xfb, 0xff, 0xb9, 0xee, 0xdb, 0xdd, 0xaf, 0xdb, 0xce, 0xbf, 0xe7,
    0xe5, 0x97, 0xdb, 0xbe, 0xe9, 0x9f, 0x9a, 0x9b, 0xab, 0xaf, 0xdc, 0x9b, 0xcb, 0x96, 0x9b, 0xa7,
    0xfb, 0xaf, 0xf2, 0xcb, 0xea, 0xf5, 0xf9, 0xce, 0x9a, 0xfc, 0xdc, 0xef, 0xbb, 0xeb, 0xfa, 0xde,
    0xb3, 0xbb, 0xb3, 0xaf, 0xbe, 0xb9, 0xbc, 0x97, 0xad, 0xcd, 0xea, 0xba, 0x9b, 0xea, 0xfe, 0xe7,
    0xf3, 0xb5, 0xd6, 0xad, 0xf9, 0xee, 0xea, 0xe6, 0xfd, 0xdb, 0x9b, 0xfd, 0xf4, 0x96, 0xdf, 0xdf,
    0xde, 0xaa, 0xeb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

static vm_segment *seg;

static void
setup()
{
    seg = vm_segment_create(_140K_);
}

static void
teardown()
{
    vm_segment_free(seg);
}

TestSuite(apple2_enc, .init = setup, .fini = teardown);

Test(apple2_enc, dos)
{
    vm_segment *enc;
    FILE *fp = fopen("../../build/karateka.dsk", "r");

    vm_segment_fread(seg, fp, 0, _140K_);
    enc = apple2_enc_dos(seg, sectab);

    cr_assert_neq(enc, NULL);

    fclose(fp);
    vm_segment_free(enc);
}

Test(apple2_enc, 4n4)
{
    int off = 0;

    off += apple2_enc_4n4(seg, off, 230);
    
    cr_assert_eq(off, 2);
    cr_assert_eq(vm_segment_get(seg, 0), 251);
    cr_assert_eq(vm_segment_get(seg, 1), 238);

    off += apple2_enc_4n4(seg, off, 123);
    cr_assert_eq(off, 4);
    cr_assert_eq(vm_segment_get(seg, 2), 191);
    cr_assert_eq(vm_segment_get(seg, 3), 251);
}

Test(apple2_enc, sector_header)
{
    apple2_enc_sector_header(seg, 0, 1, 2);

    // Check if the prologue is there
    cr_assert_eq(vm_segment_get(seg, 0), 0xd5);
    cr_assert_eq(vm_segment_get(seg, 1), 0xaa);
    cr_assert_eq(vm_segment_get(seg, 2), 0x96);

    // 4n4 of ENC_VOLUME
    cr_assert_eq(vm_segment_get(seg, 3), 0xff);
    cr_assert_eq(vm_segment_get(seg, 4), 0xfe);

    // 4n4 of the track
    cr_assert_eq(vm_segment_get(seg, 5), 0xaa);
    cr_assert_eq(vm_segment_get(seg, 6), 0xab);

    // 4n4 of the sector
    cr_assert_eq(vm_segment_get(seg, 7), 0xab);
    cr_assert_eq(vm_segment_get(seg, 8), 0xaa);

    // 4n4 of the XOR (volume ^ track ^ sector)
    cr_assert_eq(vm_segment_get(seg, 9), 0xfe);
    cr_assert_eq(vm_segment_get(seg, 10), 0xff);

    // Check if the prologue is there
    cr_assert_eq(vm_segment_get(seg, 11), 0xde);
    cr_assert_eq(vm_segment_get(seg, 12), 0xaa);
    cr_assert_eq(vm_segment_get(seg, 13), 0xeb);
}

Test(apple2_enc, sector)
{
    vm_segment *dest = vm_segment_create(1000);
    int i, len;

    vm_segment_copy_buf(seg, f_sector, 0, 0, 256);
    
    len = apple2_enc_sector(dest, seg, 0, 0);
    
    // The number of bytes (343, plus the self-sync, prologue/epilogue,
    // etc. etc.)
    cr_assert_eq(len, 382);

    // Let's see if the bytes equal what we expect for our sector
    // fixture
    for (i = 0; i < 382; i++) {
        cr_assert_eq(vm_segment_get(dest, i), f_enc_sector[i]);
    }
}

Test(apple2_enc, nib)
{
    vm_segment *seg = vm_segment_create(1000);
    vm_segment *nib;
    int i;

    for (i = 0; i < 1000; i++) {
        vm_segment_set(seg, i, 0xff);
    }

    nib = apple2_enc_nib(seg);

    for (i = 0; i < 1000; i++) {
        cr_assert_eq(vm_segment_get(nib, i), 0xff);
    }

    vm_segment_free(nib);
    vm_segment_free(seg);
}

Test(apple2_enc, track)
{
    vm_segment *dest = vm_segment_create(100000);
    int i, len;

    for (i = 0; i < ENC_NUM_SECTORS; i++) {
        vm_segment_copy_buf(seg, f_sector, i * ENC_DSECTOR, 0, 256);
    }

    apple2_enc_track(dest, seg, sectab, 0, 0);

    for (i = 0; i < ENC_ETRACK_HEADER; i++) {
        cr_assert_eq(vm_segment_get(dest, i), 0xff);
    }

    for (i = 0; i < ENC_ESECTOR; i++) {
        cr_assert_eq(
            vm_segment_get(dest, i + ENC_ETRACK_HEADER + ENC_ESECTOR_HEADER), 
            f_enc_sector[i]);
    }
    
    vm_segment_free(dest);
}
