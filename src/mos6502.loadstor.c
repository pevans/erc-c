/*
 * mos6502.loadstor.c
 */

#include "mos6502.h"
#include "mos6502.enums.h"

DEFINE_INST(lda)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, oper);
    cpu->A = oper;
}

DEFINE_INST(ldx)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, oper);
    cpu->X = oper;
}

DEFINE_INST(ldy)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, oper);
    cpu->Y = oper;
}

DEFINE_INST(pha)
{
    mos6502_push_stack(cpu, cpu->A);
}

DEFINE_INST(php)
{
    mos6502_push_stack(cpu, cpu->P);
}

DEFINE_INST(pla)
{
    cpu->A = mos6502_pop_stack(cpu);
}

DEFINE_INST(plp)
{
    cpu->P = mos6502_pop_stack(cpu);
}

DEFINE_INST(sta)
{
    vm_segment_set(cpu->memory, cpu->last_addr, cpu->A);
}

DEFINE_INST(stx)
{
    vm_segment_set(cpu->memory, cpu->last_addr, cpu->X);
}

DEFINE_INST(sty)
{
    vm_segment_set(cpu->memory, cpu->last_addr, cpu->Y);
}

DEFINE_INST(tax)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, cpu->A);
    cpu->X = cpu->A;
}

DEFINE_INST(tay)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, cpu->A);
    cpu->Y = cpu->A;
}

DEFINE_INST(tsx)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, cpu->S);
    cpu->X = cpu->S;
}

DEFINE_INST(txa)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, cpu->X);
    cpu->A = cpu->X;
}

DEFINE_INST(txs)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, cpu->X);
    cpu->S = cpu->X;
}

DEFINE_INST(tya)
{
    mos6502_modify_status(cpu, ZERO | NEGATIVE, cpu->Y);
    cpu->A = cpu->Y;
}
