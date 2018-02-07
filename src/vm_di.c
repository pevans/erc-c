/*
 * vm_di.c
 *
 * This code defines a very (very!) simplistic form of dependency
 * injection container. It's up to you to set the entries of the
 * container appropriately.
 *
 * This container is obviously _not type-safe_. It's just a bunch of
 * void pointers. This container, however, _is_ immutable; once you set
 * a value for a DI entry, it cannot be changed.
 */

#include <stdbool.h>
#include <stdlib.h>

#include "log.h"
#include "vm_di.h"

/*
 * I learned something new today: this array will be constructed with
 * zero-values for each entry because it is statically declared. See:
 * http://en.cppreference.com/w/c/language/initialization
 */
static void *di_table[VM_DI_SIZE];

#ifdef TESTING
static bool di_mutable = true;
#else
static bool di_mutable = false;
#endif

/*
 * Set the di table entry `ent` to `val`. If there is a _previous_
 * value assigned to ent, then this returns ERR_INVALID, and no
 * further assignment is allowed. 
 *
 * NOTE: in testing, we _do_ allow multiple assignments to entries, so
 * as to make it easier to tear down and rebuild machines/cpus/etc.
 * between tests.
 */
int
vm_di_set(int ent, void *val)
{
    if (di_table[ent] != NULL && !di_mutable) {
        return ERR_INVALID;
    }

    di_table[ent] = val;
    return OK;
}

/*
 * Return the value assigned to the given ent, or NULL if no entry has
 * been defined before.
 */
void *
vm_di_get(int ent)
{
    return di_table[ent];
}
