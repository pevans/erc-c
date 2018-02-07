/*
 * apple2.reflect.c
 */

#include "apple2.reflect.h"
#include "mos6502.h"

#define CPU(x) \
    mos6502 *x = (mos6502 *)ref->cpu

void
apple2_reflect_init(vm_reflect *ref)
{
}

REFLECT(apple2_reflect_cpu_info)
{
    CPU(cpu);

    printf("hey\n");
}
