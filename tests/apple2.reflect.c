#include <criterion/criterion.h>

#include "apple2.h"
#include "apple2.reflect.h"
#include "vm_di.h"
#include "vm_reflect.h"

static apple2 *mach;
static vm_reflect *ref;

static void
setup()
{
    ref = vm_reflect_create();
    vm_di_set(VM_REFLECT, ref);

    mach = apple2_create(100, 100);
    vm_di_set(VM_MACHINE, mach);
    vm_di_set(VM_CPU, mach->cpu);

    apple2_reflect_init();
}

static void
teardown()
{
    vm_reflect_free(ref);
    apple2_free(mach);

    vm_di_set(VM_REFLECT, NULL);
    vm_di_set(VM_MACHINE, NULL);
    vm_di_set(VM_CPU, NULL);
}

TestSuite(apple2_reflect, .init = setup, .fini = teardown);

Test(apple2_reflect, init)
{
    cr_assert_neq(ref->cpu_info, NULL);
    cr_assert_neq(ref->machine_info, NULL);
    cr_assert_neq(ref->pause, NULL);
    cr_assert_neq(ref->disasm, NULL);
}

/* Test(apple2_reflect, cpu_info) */
/* Test(apple2_reflect, machine_info) */

Test(apple2_reflect, pause)
{
    mach->paused = false;
    vm_reflect_pause(NULL);
    cr_assert_eq(mach->paused, true);
    vm_reflect_pause(NULL);
    cr_assert_eq(mach->paused, false);
}

Test(apple2_reflect, disasm)
{
    mach->disasm = false;
    vm_reflect_disasm(NULL);
    cr_assert_eq(mach->disasm, true);
    vm_reflect_disasm(NULL);
    cr_assert_eq(mach->disasm, false);
}
