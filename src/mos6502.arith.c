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
    MOS_CARRY_BIT();
    cpu->A += oper + carry;

    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_OVERFLOW | MOS_CARRY | MOS_ZERO, cpu->A);
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
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE | MOS_CARRY, cpu->A - oper);
}

/*
 * This instruction is functionally identical to CMP, with the exception
 * that it considers the X register rather than the accumulator.
 */
DEFINE_INST(cpx)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE | MOS_CARRY, cpu->X - oper);
}

/*
 * Again, this is a variant of the CMP instruction, except that it works
 * with the Y register.
 */
DEFINE_INST(cpy)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE | MOS_CARRY, cpu->Y - oper);
}

/*
 * Here we will decrement the value at the effective address in memory
 * by 1. The DEC instruction is _unable_ to decrement the accumulator,
 * which was a tiny oversight in the original build of the 6502.
 * (Whoopsie!)
 */
DEFINE_INST(dec) 
{
    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, oper - 1);
        mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, oper - 1);
    }
}

/*
 * In contrast, this does directly decrement the X register.
 */
DEFINE_INST(dex)
{
    cpu->X--;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->X);
}

/*
 * And, again, here we decrement the Y register.
 */
DEFINE_INST(dey)
{
    cpu->Y--;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->Y);
}

/*
 * The INC instruction is basically the same as the DEC one. It, also,
 * can only work with an address in memory, and it increments the value
 * by 1.
 */
DEFINE_INST(inc)
{
    if (cpu->eff_addr) {
        mos6502_set(cpu, cpu->eff_addr, oper + 1);
        mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, oper + 1);
    }
}

/*
 * See DEX.
 */
DEFINE_INST(inx)
{
    cpu->X++;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->X);
}

/*
 * See DEY.
 */
DEFINE_INST(iny)
{
    cpu->Y++;
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->Y);
}

/*
 * This instruction will subtract the operand from the accumulator,
 * again, "with carry". In this context, that means that if the carry
 * bit is set, then we will subtract 1 again from the A register after
 * the operand is subtracted. The result is stored in the accumulator.
 */
DEFINE_INST(sbc)
{
    MOS_CARRY_BIT();
    cpu->A = cpu->A - oper - carry;

    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_OVERFLOW | MOS_CARRY | MOS_ZERO, cpu->A);
}
