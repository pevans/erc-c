#include <criterion/criterion.h>

#include "apple2.h"

Test(apple2, create)
{
    apple2 *mach;

    mach = apple2_create();
    cr_assert_neq(mach, NULL);
    cr_assert_neq(mach->cpu, NULL);
}
