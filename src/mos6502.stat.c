/*
 * mos6502.stat.c
 */

#include "mos6502.h"
#include "mos6502.enums.h"

DEFINE_INST(clc)
{
    cpu->P &= ~CARRY;
}

DEFINE_INST(cld)
{
    cpu->P &= ~DECIMAL;
}

DEFINE_INST(cli)
{
    cpu->P &= ~INTERRUPT;
}

DEFINE_INST(clv)
{
    cpu->P &= ~OVERFLOW;
}

DEFINE_INST(sec)
{
    cpu->P |= CARRY;
}

DEFINE_INST(sed)
{
    cpu->P |= DECIMAL;
}

DEFINE_INST(sei)
{
    cpu->P |= INTERRUPT;
}
