#include <criterion/criterion.h>

#include "vm_area.h"

Test(vm_area, set)
{
    vm_area area;

    vm_area_set(&area, 1, 2, 3, 4);
    cr_assert_eq(area.xoff, 1);
    cr_assert_eq(area.yoff, 2);
    cr_assert_eq(area.width, 3);
    cr_assert_eq(area.height, 4);
}
