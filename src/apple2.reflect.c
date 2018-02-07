/*
 * apple2.reflect.c
 */

#include "apple2.h"
#include "apple2.reflect.h"
#include "mos6502.h"
#include "vm_di.h"

void
apple2_reflect_init()
{
    vm_reflect *ref = (vm_reflect *)vm_di_get(VM_REFLECT);

    ref->cpu_info = apple2_reflect_cpu_info;
    ref->machine_info = apple2_reflect_machine_info;
    ref->pause = apple2_reflect_pause;
}

REFLECT(apple2_reflect_cpu_info)
{
    mos6502 *cpu = (mos6502 *)vm_di_get(VM_CPU);
    FILE *out = (FILE *)vm_di_get(VM_OUTPUT);

    fprintf(out, "REGISTERS:\n");
    fprintf(out, "  A:%02x X:%02x Y:%02x P:%02x S:%02x PC:%04x\n",
            cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, cpu->PC);
}

REFLECT(apple2_reflect_machine_info)
{
    apple2 *mach = (apple2 *)vm_di_get(VM_MACHINE);
    FILE *out = (FILE *)vm_di_get(VM_OUTPUT);

    fprintf(out, "MACHINE:\n");
    fprintf(out, "  display_mode: %02x\n", mach->display_mode);
    fprintf(out, "  color_mode: %02x\n", mach->color_mode);
    fprintf(out, "  bank_switch: %02x\n", mach->bank_switch);
    fprintf(out, "  memory_mode: %02x\n", mach->memory_mode);
    fprintf(out, "  strobe: %02x\n", mach->strobe);
}

/*
 * If we are paused, we will unpause; if we are not paused, we will
 * pause.
 */
REFLECT(apple2_reflect_pause)
{
    apple2 *mach = (apple2 *)vm_di_get(VM_MACHINE);
    mach->paused = !mach->paused;
}