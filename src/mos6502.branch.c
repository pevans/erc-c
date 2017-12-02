/*
 * mos6502.branch.c
 */

#include "mos6502.h"
#include "mos6502.enums.h"

#define JUMP_IF(cond) \
    if (cond) cpu->PC = cpu->last_addr

DEFINE_INST(bcc)
{
    JUMP_IF(~cpu->P & CARRY);
}

DEFINE_INST(bcs)
{
    JUMP_IF(cpu->P & CARRY);
}

DEFINE_INST(beq)
{
    JUMP_IF(cpu->P & ZERO);
}

DEFINE_INST(bmi)
{
    JUMP_IF(cpu->P & NEGATIVE);
}

DEFINE_INST(bne)
{
    JUMP_IF(~cpu->P & ZERO);
}

DEFINE_INST(bpl)
{
    JUMP_IF(~cpu->P & NEGATIVE);
}

DEFINE_INST(bvc)
{
    JUMP_IF(~cpu->P & OVERFLOW);
}

DEFINE_INST(bvs)
{
    JUMP_IF(cpu->P & OVERFLOW);
}
