/*
 * mos6502.bits.c
 *
 * The code here is used to implement instructions which operate
 * specifically on bits of values.
 */

#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * The and instruction will assign the bitwise-and of the accumulator
 * and a given operand.
 */
DEFINE_INST(and)
{
    cpu->A &= oper;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->A);
}

/*
 * This is the "arithmetic" shift left instruction.
 *
 * Here we will shift the contents of the given operand left by one bit.
 * If the operand was the accumulator, then we'll store it back there;
 * if not, we will store it in the last effective address in memory.
 *
 * Note that we use the carry bit to help us figure out what the "last
 * bit" is, and whether we should now set the carry bit as a result of
 * our operation.
 */
DEFINE_INST(asl)
{
    cpu->P &= ~MOS_CARRY;
    if (oper & 0x80) {
        cpu->P |= MOS_CARRY;
    }

    oper <<= 1;

    if (cpu->last_addr) {
        mos6502_set(cpu, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }

    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_CARRY | MOS_ZERO, oper);
}

/*
 * The bit instruction will test a given operand for certain
 * characteristics, and assign the negative, overflow, and/or carry bits
 * in the status register as a result.
 */
DEFINE_INST(bit)
{
    cpu->P &= ~MOS_NEGATIVE;
    if (oper & MOS_NEGATIVE) {
        cpu->P |= MOS_NEGATIVE;
    }

    cpu->P &= ~MOS_OVERFLOW;
    if (oper & MOS_OVERFLOW) {
        cpu->P |= MOS_OVERFLOW;
    }

    if (oper & cpu->A) {
        cpu->P &= ~MOS_ZERO;
    } else {
        cpu->P |= MOS_ZERO;
    }
}

/*
 * Compute the bitwise-exclusive-or between the accumulator and operand,
 * and store the result in A.
 */
DEFINE_INST(eor)
{
    cpu->A ^= oper;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->A);
}

/*
 * This is pretty similar in spirit to the ASL instruction, except we
 * shift right rather than left.
 *
 * Note that the letters in the instruction stand for "logical" shift
 * right.
 */
DEFINE_INST(lsr)
{
    cpu->P &= ~MOS_CARRY;
    if (oper & 0x01) {
        cpu->P |= MOS_CARRY;
    }

    oper >>= 1;

    if (cpu->last_addr) {
        mos6502_set(cpu, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }

    // MOS_NEGATIVE is intentionally not included here.
    mos6502_modify_status(cpu, MOS_CARRY | MOS_ZERO, oper);
}

/*
 * Compute the bitwise-or of the accumulator and operand, and store the
 * result in the A register.
 */
DEFINE_INST(ora)
{
    cpu->A |= oper;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->A);
}

/*
 * This instruction is interesting; it's a _rotation_ left, which means
 * that what was in the 8th bit will move to the 1st bit, and everything
 * else moves down one place.
 */
DEFINE_INST(rol)
{
    MOS_CARRY_BIT();

    if (oper & 0x80) {
        carry = 1;
    }

    oper <<= 1;

    if (carry) {
        oper |= 0x01;
    }

    if (cpu->last_addr) {
        mos6502_set(cpu, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }

    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_CARRY | MOS_ZERO, oper);
}

/*
 * Here it's a rotation to the right, just like the ROL instruction. All
 * bits are maintained.
 */
DEFINE_INST(ror)
{
    MOS_CARRY_BIT();

    if (oper & 0x01) {
        carry = 1;
    }

    oper >>= 1;

    if (carry) {
        oper |= 0x80;
    }

    if (cpu->last_addr) {
        mos6502_set(cpu, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }

    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_CARRY | MOS_ZERO, oper);
}
