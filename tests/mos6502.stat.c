#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, clc)
{
    START_CPU_TEST(mos6502);

    cpu->P = CARRY | DECIMAL;
    mos6502_handle_clc(cpu, 0);
    cr_assert_eq(cpu->P & CARRY, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, cld)
{
    START_CPU_TEST(mos6502);

    cpu->P = DECIMAL | CARRY;
    mos6502_handle_cld(cpu, 0);
    cr_assert_eq(cpu->P & DECIMAL, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, cli)
{
    START_CPU_TEST(mos6502);

    cpu->P = CARRY | INTERRUPT;
    mos6502_handle_cli(cpu, 0);
    cr_assert_eq(cpu->P & INTERRUPT, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, clv)
{
    START_CPU_TEST(mos6502);

    cpu->P = CARRY | OVERFLOW;
    mos6502_handle_clv(cpu, 0);
    cr_assert_eq(cpu->P & OVERFLOW, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, sec)
{
    START_CPU_TEST(mos6502);

    cpu->P = 0;
    mos6502_handle_sec(cpu, 0);
    cr_assert_eq(cpu->P & CARRY, CARRY);

    END_CPU_TEST(mos6502);
}

Test(mos6502, sed)
{
    START_CPU_TEST(mos6502);

    cpu->P = 0;
    mos6502_handle_sed(cpu, 0);
    cr_assert_eq(cpu->P & DECIMAL, DECIMAL);

    END_CPU_TEST(mos6502);
}

Test(mos6502, sei)
{
    START_CPU_TEST(mos6502);

    cpu->P = 0;
    mos6502_handle_sei(cpu, 0);
    cr_assert_eq(cpu->P & INTERRUPT, INTERRUPT);

    END_CPU_TEST(mos6502);
}

