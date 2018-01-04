#include <criterion/criterion.h>

#include "log.h"
#include "objstore.h"

Test(objstore, init)
{
    cr_assert_eq(objstore_init(), OK);
}
