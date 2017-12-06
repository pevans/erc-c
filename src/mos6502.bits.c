/*
 * mos6502.bits.c
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
    cpu->P &= ~CARRY;
    if (oper & 0x80) {
        cpu->P |= CARRY;
    }

    oper <<= 1;

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}

/*
 * The bit instruction will test a given operand for certain
 * characteristics, and assign the negative, overflow, and/or carry bits
 * in the status register as a result.
 */
DEFINE_INST(bit)
{
    cpu->P &= ~NEGATIVE;
    if (oper & NEGATIVE) {
        cpu->P |= NEGATIVE;
    }

    cpu->P &= ~OVERFLOW;
    if (oper & OVERFLOW) {
        cpu->P |= OVERFLOW;
    }

    if (oper & cpu->A) {
        cpu->P &= ~ZERO;
    } else {
        cpu->P |= ZERO;
    }
}

/*
 * Compute the bitwise-exclusive-or between the accumulator and operand,
 * and store the result in A.
 */
DEFINE_INST(eor)
{
    cpu->A ^= oper;
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
    cpu->P &= ~CARRY;
    if (oper & 0x01) {
        cpu->P |= CARRY;
    }

    oper >>= 1;

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}

/*
 * Compute the bitwise-or of the accumulator and operand, and store the
 * result in the A register.
 */
DEFINE_INST(ora)
{
    cpu->A |= oper;
}

/*
 * This instruction is interesting; it's a _rotation_ left, which means
 * that what was in the 8th bit will move to the 1st bit, and everything
 * else moves down one place.
 */
DEFINE_INST(rol)
{
    CARRY_BIT();

    if (oper & 0x80) {
        carry = 1;
    }

    oper <<= 1;

    if (carry) {
        oper |= 0x01;
    }

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}

/*
 * Here it's a rotation to the right, just like the ROL instruction. All
 * bits are maintained.
 */
DEFINE_INST(ror)
{
    CARRY_BIT();

    if (oper & 0x01) {
        carry = 1;
    }

    oper >>= 1;

    if (carry) {
        oper |= 0x80;
    }

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}
