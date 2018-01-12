/*
 * mos6502.loadstor.c
 *
 * These are all the instructions which load and store values into
 * various registers and places in memory.
 */

#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * The LDA instruction will assign ("load") an operand into the
 * accumulator.
 */
DEFINE_INST(lda)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, oper);
    cpu->A = oper;
}

/*
 * Similar to LDA, except targeting X.
 */
DEFINE_INST(ldx)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, oper);
    cpu->X = oper;
}

/*
 * Again similar to LDA, except with Y.
 */
DEFINE_INST(ldy)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, oper);
    cpu->Y = oper;
}

/*
 * This instruction will "push" the A register onto the stack.
 */
DEFINE_INST(pha)
{
    mos6502_push_stack(cpu, cpu->A);
}

/*
 * Similar to above, but will push the P register.
 */
DEFINE_INST(php)
{
    mos6502_push_stack(cpu, cpu->P);
}

/*
 * Here we pop the stack (or "pull" it), and assign to the accumulator.
 */
DEFINE_INST(pla)
{
    cpu->A = mos6502_pop_stack(cpu);
    mos6502_modify_status(cpu, MOS_NEGATIVE | MOS_ZERO, cpu->A);
}

/*
 * Again we pop from the stack, but assign to the P register.
 */
DEFINE_INST(plp)
{
    cpu->P = mos6502_pop_stack(cpu);
}

/*
 * The STA instruction assigns the value of the accumulator to a given
 * address in memory. (That is to say, it "stores" it.)
 */
DEFINE_INST(sta)
{
    mos6502_set(cpu, cpu->eff_addr, cpu->A);
}

/*
 * Similar to STA, but drawing from the X register.
 */
DEFINE_INST(stx)
{
    mos6502_set(cpu, cpu->eff_addr, cpu->X);
}

/*
 * And, again, similar to STA, but with the Y register.
 */
DEFINE_INST(sty)
{
    mos6502_set(cpu, cpu->eff_addr, cpu->Y);
}

/*
 * The TAX instruction taxes no one but your patience for my puns. What
 * it does do is transfer the contents of the A register to X.
 */
DEFINE_INST(tax)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, cpu->A);
    cpu->X = cpu->A;
}

/*
 * This transfers from A to Y.
 */
DEFINE_INST(tay)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, cpu->A);
    cpu->Y = cpu->A;
}

/*
 * Transfer the stack pointer (S register) to X.
 */
DEFINE_INST(tsx)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, cpu->S);
    cpu->X = cpu->S;
}

/*
 * Transfer the X register to A.
 */
DEFINE_INST(txa)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, cpu->X);
    cpu->A = cpu->X;
}

/*
 * Transfer the X register to S.
 */
DEFINE_INST(txs)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, cpu->X);
    cpu->S = cpu->X;
}

/*
 * Transfer the Y register to A.
 */
DEFINE_INST(tya)
{
    mos6502_modify_status(cpu, MOS_ZERO | MOS_NEGATIVE, cpu->Y);
    cpu->A = cpu->Y;
}
