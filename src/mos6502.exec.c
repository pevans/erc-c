/*
 * mos6502.exec.c
 *
 * These instructions concern program execution; things like JMP, JSR,
 * BRK, and so forth.
 */

#include "log.h"
#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * Log the attempt to execute a "bad" (which is to say, _undefined_)
 * instruction, and exit the program. If we get to this instruction,
 * there's probably a bug in the program. (Or you're passing in an
 * invalid disk image--one of the two.)
 */
DEFINE_INST(bad)
{
    log_critical("Invalid instruction: %2x @ %4x",
                  mos6502_get(cpu, cpu->PC), cpu->PC);
    exit(1);
}

/*
 * The BRK instruction will set the interrupt bit; will push the current
 * PC address to the stack; and will advance the counter by 2 positions.
 */
DEFINE_INST(brk)
{
    mos6502_push_stack(cpu, cpu->PC >> 8);
    mos6502_push_stack(cpu, cpu->PC & 0xff);
    mos6502_push_stack(cpu, cpu->P);
    cpu->P |= MOS_INTERRUPT;
    cpu->P &= ~MOS_DECIMAL;
    cpu->PC += 2;
}

/*
 * A jump is straight forward; whatever the effective address is, that
 * is now the new value of the PC register.
 */
DEFINE_INST(jmp)
{
    cpu->PC = cpu->eff_addr;
}

/*
 * Meanwhile, a JSR (or jump to subroutine) is a little more nuanced. We
 * record our current position, plus two, to the stack, and jump the
 * effective address.
 */
DEFINE_INST(jsr)
{
    vm_16bit pc3 = cpu->PC + 2;

    mos6502_push_stack(cpu, pc3 >> 8);
    mos6502_push_stack(cpu, pc3 & 0xff);
    cpu->PC = cpu->eff_addr;
}

/*
 * The NOP instruction is short for no-operation. It does nothing except
 * waste cycles (which happens elsewhere).
 */
DEFINE_INST(nop)
{
    // do nothing
}

/*
 * Here we return from an interrupt, which effectively resets the PC
 * register to the last value on the stack.
 */
DEFINE_INST(rti)
{
    cpu->P = mos6502_pop_stack(cpu);
    cpu->PC = mos6502_pop_stack(cpu);
    cpu->PC |= mos6502_pop_stack(cpu) << 8;
}

/*
 * The RTS instruction (return from subroutine) works the same as the
 * RTI instruction, which may or may not be a misconception on my part.
 */
DEFINE_INST(rts)
{
    cpu->PC = mos6502_pop_stack(cpu);
    cpu->PC |= mos6502_pop_stack(cpu) << 8;
    cpu->PC++;
}
