#include <criterion/criterion.h>

#include "apple2.hires.h"
#include "apple2.tests.h"

TestSuite(apple2_hires, .init = setup, .fini = teardown);

Test(apple2_hires, draw)
{
}
