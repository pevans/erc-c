#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_stat, .init = setup, .fini = teardown);

Test(mos6502_stat, clc)
{
    cpu->P = CARRY | DECIMAL;
    mos6502_handle_clc(cpu, 0);
    cr_assert_eq(cpu->P & CARRY, 0);
}

Test(mos6502_stat, cld)
{
    cpu->P = DECIMAL | CARRY;
    mos6502_handle_cld(cpu, 0);
    cr_assert_eq(cpu->P & DECIMAL, 0);
}

Test(mos6502_stat, cli)
{
    cpu->P = CARRY | INTERRUPT;
    mos6502_handle_cli(cpu, 0);
    cr_assert_eq(cpu->P & INTERRUPT, 0);
}

Test(mos6502_stat, clv)
{
    cpu->P = CARRY | OVERFLOW;
    mos6502_handle_clv(cpu, 0);
    cr_assert_eq(cpu->P & OVERFLOW, 0);
}

Test(mos6502_stat, sec)
{
    cpu->P = 0;
    mos6502_handle_sec(cpu, 0);
    cr_assert_eq(cpu->P & CARRY, CARRY);
}

Test(mos6502_stat, sed)
{
    cpu->P = 0;
    mos6502_handle_sed(cpu, 0);
    cr_assert_eq(cpu->P & DECIMAL, DECIMAL);
}

Test(mos6502_stat, sei)
{
    cpu->P = 0;
    mos6502_handle_sei(cpu, 0);
    cr_assert_eq(cpu->P & INTERRUPT, INTERRUPT);
}
