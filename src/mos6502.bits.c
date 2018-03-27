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
    MOS_CHECK_NZ(cpu->A & oper);
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
    vm_8bit opcode = mos6502_get(cpu, cpu->PC);
    bool is_acc = mos6502_addr_mode(opcode) == ACC;

    vm_8bit result = oper << 1;

    MOS_CHECK_NZ(result);
    cpu->P &= ~MOS_CARRY;
    if (oper & 0x80) {
        cpu->P |= MOS_CARRY;
    }

    if (!is_acc) {
        mos6502_set(cpu, cpu->eff_addr, result);
    } else {
        cpu->A = result;
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
    MOS_CHECK_NZ(cpu->A ^ oper);
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
    vm_8bit opcode = mos6502_get(cpu, cpu->PC);
    bool is_acc = mos6502_addr_mode(opcode) == ACC;

    vm_8bit result = oper >> 1;

    // The N flag is ALWAYS cleared in LSR, because a zero is always
    // entered as bit 7
    cpu->P &= ~MOS_NEGATIVE;

    MOS_CHECK_Z(result);

    // Carry is set to the value of the bit we're "losing" in the shift
    // operation
    cpu->P &= ~MOS_CARRY;
    if (oper & 0x1) {
        cpu->P |= MOS_CARRY;
    }

    if (!is_acc) {
        mos6502_set(cpu, cpu->eff_addr, result);
    } else {
        cpu->A = result;
    }
}

/*
 * Compute the bitwise-or of the accumulator and operand, and store the
 * result in the A register.
 */
DEFINE_INST(ora)
{
    MOS_CHECK_NZ(cpu->A | oper);
    cpu->A |= oper;
}

/*
 * This instruction is interesting; it's a _rotation_ left, which means
 * that what was in the 8th bit will move to the 1st bit, and everything
 * else moves down one place.
 */
DEFINE_INST(rol)
{
    vm_8bit opcode = mos6502_get(cpu, cpu->PC);
    bool is_acc = mos6502_addr_mode(opcode) == ACC;

    vm_8bit result = oper << 1;

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

    MOS_CHECK_NZ(result);

    if (!is_acc) {
        mos6502_set(cpu, cpu->eff_addr, result);
    } else {
        cpu->A = result;
    }
}

/*
 * Here it's a rotation to the right, just like the ROL instruction. All
 * bits are maintained.
 */
DEFINE_INST(ror)
{
    vm_8bit opcode = mos6502_get(cpu, cpu->PC);
    bool is_acc = mos6502_addr_mode(opcode) == ACC;

    vm_8bit result = oper >> 1;

    // See the code for ROL for my note on 9-bit rotation (vs. 8-bit).
    if (cpu->P & MOS_CARRY) {
        result |= 0x80;
    }

    cpu->P &= ~MOS_CARRY;
    if (oper & 0x01) {
        cpu->P |= MOS_CARRY;
    }

    MOS_CHECK_NZ(result);

    if (!is_acc) {
        mos6502_set(cpu, cpu->eff_addr, result);
    } else {
        cpu->A = result;
    }
}

/*
 * This is a really funky instruction. And not in the good, dancy kinda
 * way.
 *
 * First, it does a BIT-style test to see if A & oper are zero; if so,
 * it sets the Z flag.
 *
 * Second, it clears all bits in eff_addr where A's corresponding bits
 * are set to 1. It ignores all bits in eff_addr where A's bits are
 * zero.
 *
 * E.g.:
 *
 * A: 01011001  (accumulator)
 * M: 11111111  (value in memory)
 * R: 10100110  (result)
 *
 * And, as following that, the Z flag should be zero because A&M is a
 * non-zero result.
 */
DEFINE_INST(trb)
{
    cpu->P &= ~MOS_ZERO;
    if (!(cpu->A & oper)) {
        cpu->P |= MOS_ZERO;
    }

    mos6502_set(cpu, cpu->eff_addr,
                (cpu->A ^ 0xff) & oper);
}

/*
 * Test to see if (A & oper) are zero and set Z flag if so.
 * Additionally, set the bits in the byte at a given effective address
 * (M) to 1 where the A register's bits are also 1. (Bits that are 0 in
 * A are unchanged in M.)
 */
DEFINE_INST(tsb)
{
    cpu->P &= ~MOS_ZERO;
    if (!(cpu->A & oper)) {
        cpu->P |= MOS_ZERO;
    }

    // The behavior described in the docblock here can be accomplished
    // simply by OR'ing the accumulator and the operand, and storing
    // back into memory at eff_addr.
    mos6502_set(cpu, cpu->eff_addr, cpu->A | oper);
}
