/*
 * apple2.reflect.c
 */

#include "apple2.reflect.h"
#include "mos6502.h"
#include "vm_di.h"

void
apple2_reflect_init()
{
    vm_reflect *ref = (vm_reflect *)vm_di_get(VM_REFLECT);

    ref->cpu_info = apple2_reflect_cpu_info;
}

REFLECT(apple2_reflect_cpu_info)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    FILE *out = (FILE *)vm_di_get(VM_OUTPUT);

    fprintf(out, "REGISTERS:\n");
    fprintf(out, "  A:%02x X:%02x Y:%02x P:%02x S:%02x PC:%04x\n",
            cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, cpu->PC);
}
