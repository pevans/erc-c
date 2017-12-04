#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, brk)
{
    START_CPU_TEST(mos6502);

    cpu->PC = 123;
    mos6502_handle_brk(cpu, 0);
    cr_assert_eq(cpu->PC, 125);
    cr_assert_eq(cpu->P & INTERRUPT, INTERRUPT);
    cr_assert_eq(mos6502_pop_stack(cpu), 123);

    END_CPU_TEST(mos6502);
}

Test(mos6502, jmp)
{
    START_CPU_TEST(mos6502);

    cpu->PC = 123;
    cpu->last_addr = 234;
    mos6502_handle_jmp(cpu, 0);

    cr_assert_eq(cpu->PC, 234);

    END_CPU_TEST(mos6502);
}

Test(mos6502, jsr)
{
    START_CPU_TEST(mos6502);

    cpu->PC = 123;
    cpu->last_addr = 235;
    mos6502_handle_jsr(cpu, 0);

    cr_assert_eq(cpu->PC, 235);
    cr_assert_eq(mos6502_pop_stack(cpu), 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, nop)
{
    // currently this test does nothing -- we _should_ test to see if we
    // pass the right number of cycles, though.
}

Test(mos6502, rti)
{
    START_CPU_TEST(mos6502);

    mos6502_push_stack(cpu, 222);
    mos6502_handle_rti(cpu, 0);

    cr_assert_eq(cpu->PC, 222);

    END_CPU_TEST(mos6502);
}

Test(mos6502, rts)
{
    START_CPU_TEST(mos6502);

    mos6502_push_stack(cpu, 333);
    mos6502_handle_rti(cpu, 0);

    cr_assert_eq(cpu->PC, 333);

    END_CPU_TEST(mos6502);
}
