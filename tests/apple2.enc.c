#include <criterion/criterion.h>

#include "apple2.dd.h"
#include "apple2.enc.h"
#include "vm_segment.h"

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
    enc = apple2_enc_dos(seg);

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
}
