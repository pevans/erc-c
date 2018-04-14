#include <criterion/criterion.h>

#include "apple2/apple2.h"
#include "apple2/event.h"
#include "vm_di.h"

static apple2 *mach;

static void
setup()
{
    mach = apple2_create(100, 100);

    mach->paused = false;
    mach->debug = false;

    vm_di_set(VM_PAUSE_FUNC, NULL);
    vm_di_set(VM_DEBUG_FUNC, NULL);

    apple2_event_init();
}

static void
teardown()
{
    apple2_free(mach);

    vm_di_set(VM_PAUSE_FUNC, NULL);
    vm_di_set(VM_DEBUG_FUNC, NULL);
}

TestSuite(apple2_event, .init = setup, .fini = teardown);

Test(apple2_event, init)
{
    cr_assert_neq(vm_di_get(VM_PAUSE_FUNC), NULL);
    cr_assert_neq(vm_di_get(VM_DEBUG_FUNC), NULL);
}

/* Test(apple2_reflect, cpu_info) */
/* Test(apple2_reflect, machine_info) */

Test(apple2_event, pause)
{
    apple2_event_pause(mach);
    cr_assert_eq(mach->paused, true);
    apple2_event_pause(mach);
    cr_assert_eq(mach->paused, false);
}

Test(apple2_event, debug)
{
    apple2_event_debug(mach);
    cr_assert_eq(mach->paused, true);
    cr_assert_eq(mach->debug, true);
    apple2_event_debug(mach);
    cr_assert_eq(mach->paused, true);
    cr_assert_eq(mach->debug, true);
}
