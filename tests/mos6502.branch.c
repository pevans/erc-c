#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, bcc)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bcc(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= CARRY;
    cpu->last_addr = 125;
    mos6502_handle_bcc(cpu, 0);
    cr_assert_neq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bcs)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bcs(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= CARRY;
    cpu->last_addr = 125;
    mos6502_handle_bcs(cpu, 0);
    cr_assert_eq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, beq)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_beq(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= ZERO;
    cpu->last_addr = 125;
    mos6502_handle_beq(cpu, 0);
    cr_assert_eq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bmi)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bmi(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= NEGATIVE;
    cpu->last_addr = 125;
    mos6502_handle_bmi(cpu, 0);
    cr_assert_eq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bne)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bne(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= ZERO;
    cpu->last_addr = 125;
    mos6502_handle_bne(cpu, 0);
    cr_assert_neq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bpl)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bpl(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= NEGATIVE;
    cpu->last_addr = 125;
    mos6502_handle_bpl(cpu, 0);
    cr_assert_neq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bvc)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bvc(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= OVERFLOW;
    cpu->last_addr = 125;
    mos6502_handle_bvc(cpu, 0);
    cr_assert_neq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bvs)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_bvs(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= OVERFLOW;
    cpu->last_addr = 125;
    mos6502_handle_bvs(cpu, 0);
    cr_assert_eq(cpu->PC, 125);

    END_CPU_TEST(mos6502);
}
