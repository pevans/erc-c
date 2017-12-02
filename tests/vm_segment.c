#include <criterion/criterion.h>

#include "vm_segment.h"

Test(vm_segment, create) {
    vm_segment *segment;
    int length = 128;

    segment = vm_segment_create(length);
    cr_assert_neq(segment, NULL);

    cr_assert_eq(segment->size, length);

    vm_segment_free(segment);
}

Test(vm_segment, set) {
    vm_segment *segment;
    int length = 128;
    int index = 0;
    vm_8bit value = 123;

    segment = vm_segment_create(length);
    cr_assert_neq(segment, NULL);

    vm_segment_set(segment, index, value);

    cr_assert_eq(segment->memory[index], value);
    vm_segment_free(segment);
}

Test(vm_segment, get) {
    vm_segment *segment;
    int length = 128;
    int index = 0;
    vm_8bit value = 123;

    segment = vm_segment_create(length);
    cr_assert_neq(segment, NULL);

    segment->memory[index] = value;
    cr_assert_eq(vm_segment_get(segment, index), value);

    vm_segment_free(segment);
}

Test(vm_segment, copy) {
    vm_segment *src, *dest;
    int length = 128;

    src = vm_segment_create(length);
    dest = vm_segment_create(length);

    vm_segment_set(src, 0, 0xDE);
    vm_segment_set(src, 1, 0xAD);
    vm_segment_set(src, 2, 0xBE);
    vm_segment_set(src, 3, 0xEF);

    vm_segment_copy(dest, src, 8, 0, 4);

    cr_assert_eq(vm_segment_get(dest, 8), 0xDE);
    cr_assert_eq(vm_segment_get(dest, 9), 0xAD);
    cr_assert_eq(vm_segment_get(dest, 10), 0xBE);
    cr_assert_eq(vm_segment_get(dest, 11), 0xEF);

    vm_segment_free(src);
    vm_segment_free(dest);
}
