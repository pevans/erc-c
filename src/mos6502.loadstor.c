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
    MOS_CHECK_NZ(oper);
    cpu->A = oper;
}

/*
 * Similar to LDA, except targeting X.
 */
DEFINE_INST(ldx)
{
    MOS_CHECK_NZ(oper);
    cpu->X = oper;
}

/*
 * Again similar to LDA, except with Y.
 */
DEFINE_INST(ldy)
{
    MOS_CHECK_NZ(oper);
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
 * Push the X register onto the stack. Sadly, this does not summon a
 * phoenix to assist you in hours of need.
 */
DEFINE_INST(phx)
{
    mos6502_push_stack(cpu, cpu->X);
}

/*
 * Push the Y register onto the stack
 */
DEFINE_INST(phy)
{
    mos6502_push_stack(cpu, cpu->Y);
}

/*
 * Here we pop the stack (or "pull" it), and assign to the accumulator.
 */
DEFINE_INST(pla)
{
    vm_8bit result = mos6502_pop_stack(cpu);

    MOS_CHECK_NZ(result);
    cpu->A = result;
}

/*
 * Again we pop from the stack, but assign to the P register.
 */
DEFINE_INST(plp)
{
    cpu->P = mos6502_pop_stack(cpu);
}

/*
 * Pop from the stack and assign that byte to the X register
 */
DEFINE_INST(plx)
{
    vm_8bit result = mos6502_pop_stack(cpu);

    MOS_CHECK_NZ(result);
    cpu->X = result;
}

/*
 * Pop from the stack and assign that byte to the Y register
 */
DEFINE_INST(ply)
{
    vm_8bit result = mos6502_pop_stack(cpu);

    MOS_CHECK_NZ(result);
    cpu->Y = result;
}

/*
 * The STA instruction assigns the value of the accumulator to a given
 * address in memory. (That is to say, it "stores" it.)
 */
DEFINE_INST(sta)
{
    log_debug("STA eff_addr:%04x next:%02x prev:%02x", 
              cpu->eff_addr, cpu->A, mos6502_get(cpu, cpu->eff_addr));
    mos6502_set(cpu, cpu->eff_addr, cpu->A);
}

/*
 * Similar to STA, but drawing from the X register.
 */
DEFINE_INST(stx)
{
    log_debug("STA eff_addr:%04x next:%02x prev:%02x", 
              cpu->eff_addr, cpu->X, mos6502_get(cpu, cpu->eff_addr));
    mos6502_set(cpu, cpu->eff_addr, cpu->X);
}

/*
 * And, again, similar to STA, but with the Y register.
 */
DEFINE_INST(sty)
{
    log_debug("STA eff_addr:%04x next:%02x prev:%02x", 
              cpu->eff_addr, cpu->Y, mos6502_get(cpu, cpu->eff_addr));
    mos6502_set(cpu, cpu->eff_addr, cpu->Y);
}

/*
 * Store a zero byte into the effective address
 */
DEFINE_INST(stz)
{
    mos6502_set(cpu, cpu->eff_addr, 0);
}

/*
 * The TAX instruction taxes no one but your patience for my puns. What
 * it does do is transfer the contents of the A register to X.
 */
DEFINE_INST(tax)
{
    MOS_CHECK_NZ(cpu->A);
    cpu->X = cpu->A;
}

/*
 * This transfers from A to Y.
 */
DEFINE_INST(tay)
{
    MOS_CHECK_NZ(cpu->A);
    cpu->Y = cpu->A;
}

/*
 * Transfer the stack pointer (S register) to X.
 */
DEFINE_INST(tsx)
{
    MOS_CHECK_NZ(cpu->S);
    cpu->X = cpu->S;
}

/*
 * Transfer the X register to A.
 */
DEFINE_INST(txa)
{
    MOS_CHECK_NZ(cpu->X);
    cpu->A = cpu->X;
}

/*
 * Transfer the X register to S.
 */
DEFINE_INST(txs)
{
    MOS_CHECK_NZ(cpu->X);
    cpu->S = cpu->X;
}

/*
 * Transfer the Y register to A.
 */
DEFINE_INST(tya)
{
    MOS_CHECK_NZ(cpu->Y);
    cpu->A = cpu->Y;
}
