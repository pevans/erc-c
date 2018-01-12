#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_exec, .init = setup, .fini = teardown);

Test(mos6502_exec, brk)
{
    vm_8bit orig_P = cpu->P;

    cpu->PC = 123;
    mos6502_handle_brk(cpu, 0);
    cr_assert_eq(cpu->PC, 125);
    cr_assert_eq(cpu->P & MOS_INTERRUPT, MOS_INTERRUPT);

    cr_assert_eq(mos6502_pop_stack(cpu), orig_P);
    cr_assert_eq(mos6502_pop_stack(cpu), 123);
}

Test(mos6502_exec, jmp)
{
    cpu->PC = 123;
    cpu->eff_addr = 234;
    mos6502_handle_jmp(cpu, 0);

    cr_assert_eq(cpu->PC, 234);
}

Test(mos6502_exec, jsr)
{
    cpu->PC = 123;
    cpu->eff_addr = 235;
    mos6502_handle_jsr(cpu, 0);

    cr_assert_eq(cpu->PC, 235);
    
    cr_assert_eq(mos6502_pop_stack(cpu), 126);
}

Test(mos6502_exec, nop)
{
    // currently this test does nothing -- we _should_ test to see if we
    // pass the right number of cycles, though.
}

Test(mos6502_exec, rti)
{
    mos6502_push_stack(cpu, 222);
    mos6502_push_stack(cpu, cpu->P);

    mos6502_handle_rti(cpu, 0);

    cr_assert_eq(cpu->PC, 222);
}

Test(mos6502_exec, rts)
{
    mos6502_push_stack(cpu, 333);
    mos6502_handle_rts(cpu, 0);

    cr_assert_eq(cpu->PC, 333);
}
