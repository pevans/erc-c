/*
 * mos6502.stat.c
 *
 * The "stat", here, is short for status; these instructions all
 * directly modify the status (P) register.
 */

#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * Clear the carry bit in the status register.
 */
DEFINE_INST(clc)
{
    cpu->P &= ~CARRY;
}

/*
 * Clear the decimal bit.
 */
DEFINE_INST(cld)
{
    cpu->P &= ~DECIMAL;
}

/*
 * Clear the interrupt bit.
 */
DEFINE_INST(cli)
{
    cpu->P &= ~INTERRUPT;
}

/*
 * Clear the overflow bit.
 */
DEFINE_INST(clv)
{
    cpu->P &= ~OVERFLOW;
}

/*
 * Set the carry bit.
 */
DEFINE_INST(sec)
{
    cpu->P |= CARRY;
}

/*
 * Set the decimal bit.
 */
DEFINE_INST(sed)
{
    cpu->P |= DECIMAL;
}

/*
 * Set the interrupt bit.
 */
DEFINE_INST(sei)
{
    cpu->P |= INTERRUPT;
}
