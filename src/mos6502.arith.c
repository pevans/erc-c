/*
 * mos6502.arith.c
 *
 * We define here the logic for arithmetic instructions for the MOS 6502
 * processor.
 */

#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * The adc instruction will add a number to the accumulator, "with
 * carry". If the carry bit is set, we will add 1 to the accumulator as
 * an after-effect.
 */
DEFINE_INST(adc)
{
    if (cpu->P & MOS_DECIMAL) {
        mos6502_handle_adc_dec(cpu, oper);
        return;
    }

    MOS_CARRY_BIT();

    vm_8bit result = cpu->A + oper + carry;
    MOS_CHECK_NZ(result);

    cpu->P &= ~MOS_OVERFLOW;
    if (((cpu->A ^ oper) & 0x80) &&
        ((cpu->A ^ result) & 0x80)
       ) {
        cpu->P |= MOS_OVERFLOW;
    }

    // Carry has different meanings in different contexts... in ADC,
    // carry is set if the result requires a ninth bit (the carry bit!)
    // to be set.
    cpu->P &= ~MOS_CARRY;
    if ((cpu->A + oper + carry) > 0xff) {
        cpu->P |= MOS_CARRY;
    }
    
    cpu->A = result;
}

/*
 * Add a number to the accumulator using Binary-Coded Decimal, or BCD.
 * Some things still work the same--for instance, if carry is high, we
 * will still add one to A+M. The V flag doesn't make any sense in BCD,
 * and apparently ADC's effects on V in decimal mode is "undocumented"?
 * The N flag is also a bit odd, and the general wisdom seems to be that
 * you should use multiple bytes if you want to represent a negative,
 * and just use the sign bit on the MSB.
 */
DEFINE_INST(adc_dec)
{
    MOS_CARRY_BIT();

    // Determine the most and least siginificant digits of A and oper.
    int a_msd = cpu->A >> 4,
        a_lsd = cpu->A & 0xf,
        o_msd = oper >> 4,
        o_lsd = oper & 0xf;
    
    // If any of these are greater than 9, then they are invalid BCD
    // values, and we give up.
    if (a_msd > 9 || a_lsd > 9 || o_msd > 9 || o_lsd > 9) {
        return;
    }

    // Sum is built using the decimal senses of the msd/lsd variables;
    // carry is also a factor.
    int sum =
        ((a_msd * 10) + a_lsd) +
        ((o_msd * 10) + o_lsd) + carry;

    // But ultimately, one byte can only hold $00 - $99
    int modsum = sum % 100;

    // And the final result has to be ported back into a "hexadecimal"
    // number; you see, BCD values are not just literally decimal, they
    // are decimal in hexadecimal form.
    vm_8bit result = ((modsum / 10) << 4) | (modsum % 10);

    cpu->P &= ~MOS_OVERFLOW;
    if (((cpu->A ^ sum) & 0x80) &&
        ((cpu->A ^ result) & 0x80)
       ) {
        cpu->P |= MOS_OVERFLOW;
    }

    // As you can see, decimal comports a different meaning for the
    // carry bit than its binary version
    cpu->P &= ~MOS_CARRY;
    if (sum > 100) {
        cpu->P |= MOS_CARRY;
    }

    MOS_CHECK_Z(result);

    cpu->A = result;
}

/*
 * The cmp instruction will consider the difference of the accumulator
 * minus the operand. It will then set the zero, negative, or carry bits
 * based upon that difference. _The accumulator is neither modified nor
 * harmed by this operation._ (We have trained experts on the set to
 * monitor the health of the accumulator, whom we've named George.)
 */
DEFINE_INST(cmp)
{
    MOS_CHECK_NZ(cpu->A - oper);

    // With CMP, carry is set if the difference between A and oper is
    // not zero.
    cpu->P &= ~MOS_CARRY;
    if (cpu->A >= oper) {
        cpu->P |= MOS_CARRY;
    }
}

/*
 * This instruction is functionally identical to CMP, with the exception
 * that it considers the X register rather than the accumulator.
 */
DEFINE_INST(cpx)
{
    MOS_CHECK_NZ(cpu->X - oper);

    cpu->P &= ~MOS_CARRY;
    if (cpu->X >= oper) {
        cpu->P |= MOS_CARRY;
    }
}

/*
 * Again, this is a variant of the CMP instruction, except that it works
 * with the Y register.
 */
DEFINE_INST(cpy)
{
    MOS_CHECK_NZ(cpu->Y - oper);

    cpu->P &= ~MOS_CARRY;
    if (cpu->Y >= oper) {
        cpu->P |= MOS_CARRY;
    }
}

/*
 * Here we will decrement the value at the effective address in memory
 * by 1.
 */
DEFINE_INST(dec) 
{
    vm_8bit opcode = mos6502_get(cpu, cpu->PC);
    bool is_acc = mos6502_addr_mode(opcode) == ACC;

    if (!is_acc) {
        MOS_CHECK_NZ(oper - 1);
        mos6502_set(cpu, cpu->eff_addr, oper - 1);
        return;
    }

    // If we get here, then this is ACC mode, and we should work off
    // that.
    MOS_CHECK_NZ(cpu->A - 1);
    cpu->A--;
}

/*
 * In contrast, this does directly decrement the X register.
 */
DEFINE_INST(dex)
{
    MOS_CHECK_NZ(cpu->X - 1);
    cpu->X--;
}

/*
 * And, again, here we decrement the Y register.
 */
DEFINE_INST(dey)
{
    MOS_CHECK_NZ(cpu->Y - 1);
    cpu->Y--;
}

/*
 * The INC instruction is basically the same as the DEC one.
 */
DEFINE_INST(inc)
{
    vm_8bit opcode = mos6502_get(cpu, cpu->PC);
    bool is_acc = mos6502_addr_mode(opcode) == ACC;

    if (!is_acc) {
        MOS_CHECK_NZ(oper + 1);
        mos6502_set(cpu, cpu->eff_addr, oper + 1);
        return;
    }

    MOS_CHECK_NZ(cpu->A + 1);
    cpu->A++;
}

/*
 * See DEX.
 */
DEFINE_INST(inx)
{
    MOS_CHECK_NZ(cpu->X + 1);
    cpu->X++;
}

/*
 * See DEY.
 */
DEFINE_INST(iny)
{
    MOS_CHECK_NZ(cpu->Y + 1);
    cpu->Y++;
}

/*
 * This instruction will subtract the operand from the accumulator,
 * again, "with carry". In this context, that means that if the carry
 * bit is set, then we will subtract 1 again from the A register after
 * the operand is subtracted. The result is stored in the accumulator.
 */
DEFINE_INST(sbc)
{
    // Jump into the binary coded decimal world with us! It's... funky!
    if (cpu->P & MOS_DECIMAL) {
        mos6502_handle_sbc_dec(cpu, oper);
        return;
    }

    MOS_CARRY_BIT();

    int result32 = cpu->A - oper - (carry ? 0 : 1);
    vm_8bit result8 = cpu->A - oper - (carry ? 0 : 1);

    MOS_CHECK_Z(result8);

    // Carry is handled slightly differently in SBC; it's set if the
    // value is non-negative, and unset if negative. (It's essentially a
    // mirror of the N flag in that sense.)
    cpu->P |= MOS_CARRY;
    cpu->P &= ~MOS_NEGATIVE;

    if (result32 < 0) {
        cpu->P &= ~MOS_CARRY;
        cpu->P |= MOS_NEGATIVE;
    }

    cpu->P &= ~MOS_OVERFLOW;
    if (((cpu->A ^ oper) & 0x80) &&
        ((cpu->A ^ result8) & 0x80)
       ) {
        cpu->P |= MOS_OVERFLOW;
    }

    cpu->A = result8;
}

/*
 * Pretty similar to the code we're doing in adc_dec; we are here
 * performing a subtraction in binary coded decimal.
 *
 * Note: a lot of this code is lifted from adc_dec; it's probably the
 * only other time we will need to use this specific code, so I'm doing
 * the Martin Fowler thing and waiting for a third occasion to arise
 * before refactoring this into its own function.
 */
DEFINE_INST(sbc_dec)
{
    MOS_CARRY_BIT();

    // Determine the most and least siginificant digits of A and oper.
    int a_msd = cpu->A >> 4,
        a_lsd = cpu->A & 0xf,
        o_msd = oper >> 4,
        o_lsd = oper & 0xf;
    
    // If any of these are greater than 9, then they are invalid BCD
    // values, and we give up.
    if (a_msd > 9 || a_lsd > 9 || o_msd > 9 || o_lsd > 9) {
        return;
    }

    // Sum is built using the decimal senses of the msd/lsd variables;
    // carry is also a factor.
    int diff =
        ((a_msd * 10) + a_lsd) -
        ((o_msd * 10) + o_lsd) - (carry ? 0 : 1);

    // Force C to high to begin with
    cpu->P |= MOS_CARRY;

    // If diff is negative, we need to "overflow" it back to a
    // positive number by adding 100. We also need to unset the C flag.
    if (diff < 0) {
        diff += 100;
        cpu->P &= ~MOS_CARRY;
    }

    // And the final result has to be ported back into a "hexadecimal"
    // number; you see, BCD values are not just literally decimal, they
    // are decimal in hexadecimal form.
    vm_8bit result = ((diff / 10) << 4) | (diff % 10);
    MOS_CHECK_Z(result);

    // This is a bit of an odd sequence... but overflow in decimal is a
    // bit odd to begin with. I ended up replicating the behavior of
    // AppleWin here.
    cpu->P &= ~MOS_OVERFLOW;
    if (((cpu->A ^ diff) & 0x80) &&
        ((cpu->A ^ result) & 0x80)
       ) {
        cpu->P |= MOS_OVERFLOW;
    }

    cpu->A = result;
}
