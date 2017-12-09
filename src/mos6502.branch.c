/*
 * mos6502.branch.c
 *
 * This is all the logic we use for branch instructions, which are used
 * for conditional expressions.
 */

#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * This is just a minor convenience macro to wrap the logic we use in
 * branch situations, which is if `cond` is true, then we set the
 * program counter to the last effective address.
 */
#define JUMP_IF(cond) \
    if (cond) cpu->PC = cpu->last_addr

/*
 * Branch if the carry flag is clear.
 */
DEFINE_INST(bcc)
{
    JUMP_IF(~cpu->P & CARRY);
}

/*
 * Branch if carry is set.
 */
DEFINE_INST(bcs)
{
    JUMP_IF(cpu->P & CARRY);
}

/*
 * Branch if the zero flag is set (that is, if our last instruction
 * resulted in something being _equal to zero_).
 */
DEFINE_INST(beq)
{
    JUMP_IF(cpu->P & ZERO);
}

/*
 * Branch if the negative ("minus") flag is set.
 */
DEFINE_INST(bmi)
{
    JUMP_IF(cpu->P & NEGATIVE);
}

/*
 * Branch if the zero flag is not set; which is to say, that the last
 * operation was _not equal_ to zero.
 */
DEFINE_INST(bne)
{
    JUMP_IF(~cpu->P & ZERO);
}

/*
 * Branch if the negative flag is not set (meaning the last operation
 * was "plus", which includes zero).
 */
DEFINE_INST(bpl)
{
    JUMP_IF(~cpu->P & NEGATIVE);
}

/*
 * Branch if the overflow bit is clear.
 */
DEFINE_INST(bvc)
{
    JUMP_IF(~cpu->P & OVERFLOW);
}

/*
 * Branch if the overflow bit is set.
 */
DEFINE_INST(bvs)
{
    JUMP_IF(cpu->P & OVERFLOW);
}
