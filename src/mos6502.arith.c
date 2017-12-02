/*
 * mos6502.inst.c
 */

#include "mos6502.h"
#include "mos6502.enums.h"

DEFINE_INST(adc)
{
    CARRY_BIT();
    cpu->A += oper + carry;
}

DEFINE_INST(cmp)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE | CARRY, cpu->A - oper);
}

DEFINE_INST(cpx)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE | CARRY, cpu->X - oper);
}

DEFINE_INST(cpy)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE | CARRY, cpu->Y - oper);
}

DEFINE_INST(dec) 
{
    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper - 1);
    }
}

DEFINE_INST(dex)
{
    cpu->X--;
}

DEFINE_INST(dey)
{
    cpu->Y--;
}

DEFINE_INST(inc)
{
    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper + 1);
    }
}

DEFINE_INST(inx)
{
    cpu->X++;
}

DEFINE_INST(iny)
{
    cpu->Y++;
}

DEFINE_INST(sbc)
{
    CARRY_BIT();
    cpu->A -= oper - carry;
}
