#include <criterion/criterion.h>

#include "apple2.h"
#include "vm_reflect.h"

static vm_reflect *ref;
static apple2 *mach;

/*
 * This is a tiny function we can use that will satisfy the
 * vm_reflect_fn type so we can test that all of those are working in
 * the vm_reflect struct.
 */
static void
fun(vm_reflect *ref)
{
    return;
}

static void
have_fun()
{
    ref->cpu_info = fun;
    ref->machine_info = fun;
    ref->pause = fun;
    ref->resume = fun;
    ref->disasm_on = fun;
    ref->disasm_off = fun;
}

static void
setup()
{
    mach = apple2_create(100, 100);
    ref = vm_reflect_create(mach, mach->cpu, stdout);
}

static void
teardown()
{
    vm_reflect_free(ref);
    apple2_free(mach);
}

TestSuite(vm_reflect, .init = setup, .fini = teardown);

Test(vm_reflect, create)
{
    cr_assert_neq(ref, NULL);
    cr_assert_eq(ref->cpu_info, NULL);
    cr_assert_eq(ref->machine_info, NULL);
    cr_assert_eq(ref->pause, NULL);
    cr_assert_eq(ref->resume, NULL);
    cr_assert_eq(ref->disasm_on, NULL);
    cr_assert_eq(ref->disasm_off, NULL);
}

// Not much to do here
/* Test(vm_reflect, free) */

/*
 * Because all of the reflect functions (aside from create/free) do the
 * same thing, one test can stand in for all of them; hence the skips
 * you see below.
 */
Test(vm_reflect, cpu_info)
{
    // We should always try to have fun
    have_fun();

    cr_assert_eq(vm_reflect_cpu_info(ref), OK);
}

/* Test(vm_reflect, machine_info) */
/* Test(vm_reflect, pause) */
/* Test(vm_reflect, resume) */
/* Test(vm_reflect, disasm_on) */
/* Test(vm_reflect, disasm_off) */
