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
