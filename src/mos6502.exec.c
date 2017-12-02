/*
 * mos6502.exec.c
 */

#include "mos6502.h"
#include "mos6502.enums.h"

DEFINE_INST(brk)
{
    cpu->P |= INTERRUPT;
    mos6502_push_stack(cpu, cpu->PC);
    cpu->PC += 2;
}

DEFINE_INST(jmp)
{
    cpu->PC = cpu->last_addr;
}

DEFINE_INST(jsr)
{
    mos6502_push_stack(cpu, cpu->PC + 2);
    cpu->PC = cpu->last_addr;
}

DEFINE_INST(nop)
{
    // do nothing
}

DEFINE_INST(rti)
{
    cpu->PC = mos6502_pop_stack(cpu);
}

DEFINE_INST(rts)
{
    cpu->PC = mos6502_pop_stack(cpu);
}
