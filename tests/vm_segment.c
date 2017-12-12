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
read_fn(vm_segment *segment, size_t addr)
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
write_fn(vm_segment *segment, size_t addr, vm_8bit value)
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
