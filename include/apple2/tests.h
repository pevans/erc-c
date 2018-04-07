#ifndef _APPLE2_TESTS_H_
#define _APPLE2_TESTS_H_

#include "apple2.h"
#include "vm_segment.h"

static apple2 *mach = NULL;

static void
setup()
{
    mach = apple2_create(100, 100);
    vm_segment_set_map_machine(mach);
}

static void
teardown()
{
    apple2_free(mach);
    vm_segment_set_map_machine(NULL);
}

#endif
