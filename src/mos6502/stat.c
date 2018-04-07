/*
 * mos6502.stat.c
 *
 * The "stat", here, is short for status; these instructions all
 * directly modify the status (P) register.
 */

#include "mos6502/mos6502.h"
#include "mos6502/enums.h"

/*
 * Clear the carry bit in the status register.
 */
DEFINE_INST(clc)
{
    cpu->P &= ~MOS_CARRY;
}

/*
 * Clear the decimal bit.
 */
DEFINE_INST(cld)
{
    cpu->P &= ~MOS_DECIMAL;
}

/*
 * Clear the interrupt bit.
 */
DEFINE_INST(cli)
{
    cpu->P &= ~MOS_INTERRUPT;
}

/*
 * Clear the overflow bit.
 */
DEFINE_INST(clv)
{
    cpu->P &= ~MOS_OVERFLOW;
}

/*
 * Set the carry bit.
 */
DEFINE_INST(sec)
{
    cpu->P |= MOS_CARRY;
}

/*
 * Set the decimal bit.
 */
DEFINE_INST(sed)
{
    cpu->P |= MOS_DECIMAL;
}

/*
 * Set the interrupt bit.
 */
DEFINE_INST(sei)
{
    cpu->P |= MOS_INTERRUPT;
}
