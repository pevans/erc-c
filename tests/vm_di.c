#include <criterion/criterion.h>

#include "vm_di.h"

/*
 * The set test is also (informally) the get test, since there's no
 * independent means to inspect what is in the DI container table.
 *
 * Test(vm_di, get)
 */
Test(vm_di, set)
{
    cr_assert_eq(vm_di_get(VM_MACHINE), NULL);
    vm_di_set(VM_MACHINE, (void *)123);
    cr_assert_eq(vm_di_get(VM_MACHINE), (void *)123);
    vm_di_set(VM_MACHINE, (void *)234);
    cr_assert_eq(vm_di_get(VM_MACHINE), (void *)234);
}
