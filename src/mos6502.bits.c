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
 * The BIM instruction (which is made up--it's not a real instruction)
 * is here to handle the specific use-case of a BIT instruction in
 * immediate (IMM) mode. We do this in a separate instruction to avoid
 * the need to add logic to the BIT instruction such that it has to know
 * or care about its opcode or its address mode.
 */
DEFINE_INST(bim)
{
    // This is the same behavior as BIT
    cpu->P &= ~MOS_ZERO;
    if (!(cpu->A & oper)) {
        cpu->P |= MOS_ZERO;
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

    // Set ZERO if it should apply; also inspect the NEGATIVE flag, but
    // note that doing so will have the effect of _always_ unsetting the
    // N flag, because literally any byte value shifted right will
    // immediately lose the sign bit and be non-negative.
    mos6502_modify_status(cpu, MOS_NZ, oper, result);

    // However, we handle carry a bit differently here. The carry bit
    // should be 1 if oper & 0x1; that is, when we shift right, we want
    // the right-most bit to be captured in carry, in just the same way
    // we want the left-most bit to be captured in carry for ASL.
    cpu->P &= ~MOS_CARRY;
    if (oper & 0x1) {
        cpu->P |= MOS_CARRY;
    }

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
    SET_RESULT(oper << 1);

    // Rotations are effectively _9-bit_. So we aren't rotating bit 7
    // into bit 0; we're rotating bit 7 into the carry bit, and we're
    // rotating the _previous value of the carry bit_ into bit 0.
    if (cpu->P & MOS_CARRY) {
        result |= 0x1;
    }

    cpu->P &= ~MOS_CARRY;
    if (oper & 0x80) {
        cpu->P |= MOS_CARRY;
    }

    mos6502_modify_status(cpu, MOS_NZ, oper, result);

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
    SET_RESULT(oper >> 1);

    // See the code for ROL for my note on 9-bit rotation (vs. 8-bit).
    if (cpu->P & MOS_CARRY) {
        result |= 0x80;
    }

    cpu->P &= ~MOS_CARRY;
    if (oper & 0x01) {
        cpu->P |= MOS_CARRY;
    }

    mos6502_modify_status(cpu, MOS_NZ, oper, result);

    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, result & 0xff);
    } else {
        cpu->A = result & 0xff;
    }
}
