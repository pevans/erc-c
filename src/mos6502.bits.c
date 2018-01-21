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
    SET_RESULT(cpu->A & oper);

    mos6502_modify_status(cpu, MOS_NZ, cpu->A, result);
    cpu->A = result & 0xff;
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
    SET_RESULT(oper << 1);

    mos6502_modify_status(cpu, MOS_NZC, oper, result);

    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, result & 0xff);
    } else {
        cpu->A = result & 0xff;
    }
}

/*
 * The bit instruction will test a given operand for certain
 * characteristics, and assign the negative, overflow, and/or carry bits
 * in the status register as a result.
 */
DEFINE_INST(bit)
{
    // Zero is set if the accumulator AND the operand results in zero.
    cpu->P &= ~MOS_ZERO;
    if (!(cpu->A & oper)) {
        cpu->P |= MOS_ZERO;
    }

    // But negative is set not by any operation on the accumulator; it
    // is, rather, set by evaluating the operand itself.
    cpu->P &= ~MOS_NEGATIVE;
    if (oper & 0x80) {
        cpu->P |= MOS_NEGATIVE;
    }

    // Normally, overflow is handled by checking if bit 7 flipped from 0
    // to 1 or vice versa, and that's done by comparing the result to
    // the operand. But in the case of BIT, all we want to know is if
    // bit 6 is high.
    cpu->P &= ~MOS_OVERFLOW;
    if (oper & 0x40) {
        cpu->P |= MOS_OVERFLOW;
    }
}

/*
 * Compute the bitwise-exclusive-or between the accumulator and operand,
 * and store the result in A.
 */
DEFINE_INST(eor)
{
    SET_RESULT(cpu->A ^ oper);

    mos6502_modify_status(cpu, MOS_NZ, cpu->A, result);
    cpu->A = result & 0xff;
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
    SET_RESULT(oper >> 1);

    // MOS_NEGATIVE is intentionally not included here.
    mos6502_modify_status(cpu, MOS_ZC, oper, result);

    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, result & 0xff);
    } else {
        cpu->A = result & 0xff;
    }
}

/*
 * Compute the bitwise-or of the accumulator and operand, and store the
 * result in the A register.
 */
DEFINE_INST(ora)
{
    SET_RESULT(cpu->A | oper);

    mos6502_modify_status(cpu, MOS_NZ, cpu->A, result);
    cpu->A = result & 0xff;
}

/*
 * This instruction is interesting; it's a _rotation_ left, which means
 * that what was in the 8th bit will move to the 1st bit, and everything
 * else moves down one place.
 */
DEFINE_INST(rol)
{
    MOS_CARRY_BIT();
    SET_RESULT(oper << 1);

    if (oper & 0x80) {
        carry = 1;
    }

    if (carry) {
        result |= 0x01;
    }

    mos6502_modify_status(cpu, MOS_NZC, oper, result);

    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, result & 0xff);
    } else {
        cpu->A = result & 0xff;
    }
}

/*
 * Here it's a rotation to the right, just like the ROL instruction. All
 * bits are maintained.
 */
DEFINE_INST(ror)
{
    MOS_CARRY_BIT();
    SET_RESULT(oper >> 1);

    if (oper & 0x01) {
        carry = 1;
    }

    if (carry) {
        result |= 0x80;
    }

    mos6502_modify_status(cpu, MOS_NZC, oper, result);

    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, result & 0xff);
    } else {
        cpu->A = result & 0xff;
    }
}
