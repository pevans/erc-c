#include <criterion/criterion.h>

#include "vm_segment.h"

static vm_segment *segment;
static int length = 128;

static void
setup()
{
    segment = vm_segment_create(length);
}

static void
teardown()
{
    vm_segment_free(segment);
}

TestSuite(vm_segment, .init = setup, .fini = teardown);

/* Test(vm_segment, free) */
Test(vm_segment, create) {
    int i;

    cr_assert_neq(segment, NULL);
    cr_assert_eq(segment->size, length);

    // Test that the memory chunk itself, plus the read and write
    // tables, are all zeroed out.
    for (i = 0; i < segment->size; i++) {
        cr_assert_eq(segment->memory[i], 0);
        cr_assert_eq(segment->read_table[i], NULL);
        cr_assert_eq(segment->write_table[i], NULL);
    }
}

Test(vm_segment, set) {
    int index = 0;
    vm_8bit value = 123;

    cr_assert_eq(vm_segment_set(segment, index, value), OK);

    cr_assert_eq(segment->memory[index], value);
}

Test(vm_segment, get) {
    int index = 0;
    vm_8bit value = 123;

    segment->memory[index] = value;
    cr_assert_eq(vm_segment_get(segment, index), value);
}

Test(vm_segment, copy) {
    vm_segment *src, *dest;

    src = vm_segment_create(length);
    dest = vm_segment_create(length);

    vm_segment_set(src, 0, 0xDE);
    vm_segment_set(src, 1, 0xAD);
    vm_segment_set(src, 2, 0xBE);
    vm_segment_set(src, 3, 0xEF);

    cr_assert_eq(vm_segment_copy(dest, src, 8, 0, 4), OK);

    cr_assert_eq(vm_segment_get(dest, 8), 0xDE);
    cr_assert_eq(vm_segment_get(dest, 9), 0xAD);
    cr_assert_eq(vm_segment_get(dest, 10), 0xBE);
    cr_assert_eq(vm_segment_get(dest, 11), 0xEF);

    vm_segment_free(src);
    vm_segment_free(dest);
}

Test(vm_segment, read_map)
{
    // We use some trickery here; I don't want to go to the trouble of
    // defining a function for this test, so I am just passing in an
    // arbitrary pointer to junk. As long as the garbage-in, garbage-out
    // principle holds, we're golden.
    cr_assert_eq(vm_segment_read_map(segment, 123, (vm_segment_read_fn)456), OK);
    cr_assert_eq(vm_segment_read_map(segment, 321, (vm_segment_read_fn)456), ERR_OOB);

    cr_assert_eq(segment->read_table[123], (vm_segment_read_fn)456);
}

Test(vm_segment, write_map)
{
    // See the read_map test for more details on the following trickery
    cr_assert_eq(vm_segment_write_map(segment, 123, (vm_segment_write_fn)456), OK);
    cr_assert_eq(vm_segment_write_map(segment, 321, (vm_segment_write_fn)456), ERR_OOB);

    cr_assert_eq(segment->write_table[123], (vm_segment_write_fn)456);
}

static vm_8bit
read_fn(vm_segment *segment, size_t addr, void *_mach)
{
    return 222;
}

Test(vm_segment, use_read_map)
{
    size_t addr = 123;

    vm_segment_set(segment, addr, 111);
    cr_assert_eq(vm_segment_get(segment, addr), 111);
    vm_segment_read_map(segment, addr, read_fn);
    cr_assert_eq(vm_segment_get(segment, addr), 222);
    vm_segment_read_map(segment, addr, NULL);
    cr_assert_eq(vm_segment_get(segment, addr), 111);
}

void
write_fn(vm_segment *segment, size_t addr, vm_8bit value, void *_mach)
{
    segment->memory[addr+1] = value;
}

Test(vm_segment, use_write_map)
{
    size_t addr = 123;

    vm_segment_set(segment, addr, 111);
    cr_assert_eq(vm_segment_get(segment, addr), 111);
    cr_assert_neq(vm_segment_get(segment, addr + 1), 111);
    vm_segment_write_map(segment, addr, write_fn);
    vm_segment_set(segment, addr, 111);
    cr_assert_eq(vm_segment_get(segment, addr), 111);
    cr_assert_eq(vm_segment_get(segment, addr + 1), 111);
}

Test(vm_segment, copy_buf)
{
    vm_8bit buf[] = {1, 2, 3, 4, 5};

    vm_segment_copy_buf(segment, buf, 0, 0, 3);

    cr_assert_eq(vm_segment_get(segment, 0), 1);
    cr_assert_eq(vm_segment_get(segment, 1), 2);
    cr_assert_eq(vm_segment_get(segment, 2), 3);

    // Note that segments by default are zeroed out, so we can safely
    // assume its original values should not be 4 or 5 at these indexes.
    cr_assert_neq(vm_segment_get(segment, 3), 4);
    cr_assert_neq(vm_segment_get(segment, 4), 5);
}

Test(vm_segment, fread)
{
    FILE *stream;

    stream = fopen("../data/zero.img", "r");
    cr_assert_eq(vm_segment_fread(segment, stream, 0, 123), OK);
}

Test(vm_segment, get16)
{
    vm_segment_set(segment, 0, 0x34);
    vm_segment_set(segment, 1, 0x12);

    cr_assert_eq(vm_segment_get16(segment, 0), 0x1234);
}

Test(vm_segment, set16)
{
    vm_segment_set16(segment, 0, 0x2345);
    cr_assert_eq(vm_segment_get16(segment, 0), 0x2345);
}

Test(vm_segment, fwrite)
{
    FILE *stream;

    stream = fopen("../data/zero.img", "r");
    vm_segment_fread(segment, stream, 0, 123);

    fclose(stream);
    stream = fopen("/tmp/zero.img", "w");
    cr_assert_eq(vm_segment_fwrite(segment, stream, 0, 123), OK);

    fclose(stream);
}

Test(vm_segment, hexdump)
{
    vm_segment_set(segment, 0, 'H');
    vm_segment_set(segment, 1, 'e');
    vm_segment_set(segment, 2, 'l');
    vm_segment_set(segment, 3, 'l');
    vm_segment_set(segment, 4, 'o');
    vm_segment_set(segment, 5, ' ');
    vm_segment_set(segment, 6, 'N');
    vm_segment_set(segment, 7, 'e');
    vm_segment_set(segment, 8, 'r');
    vm_segment_set(segment, 9, 'd');
    vm_segment_set(segment, 10, 's');

    FILE *stream = fopen("/dev/null", "w");
    char buf[512];

    setvbuf(stream, buf, _IOFBF, BUFSIZ);
    vm_segment_hexdump(segment, stream, 0, 16);

    cr_assert_str_eq(buf, 
                     "00000000    48 65 6C 6C 6F 20 4E 65  72 64 73 00 00 00 00 00   [Hello Nerds.....]\n");
}
