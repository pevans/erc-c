#include <criterion/criterion.h>

#include "log.h"
#include "objstore.h"

Test(objstore, init)
{
    objstore_clear();
    cr_assert_eq(objstore_init(), OK);
    cr_assert_eq(objstore_ready(), true);
}

Test(objstore, ready)
{
    objstore_clear();
    cr_assert_eq(objstore_ready(), false);
    objstore_init();
    cr_assert_eq(objstore_ready(), true);
}

Test(objstore, clear)
{
    objstore_init();
    cr_assert_eq(objstore_ready(), true);
    objstore_clear();
    cr_assert_eq(objstore_ready(), false);
}
