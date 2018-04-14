#include <criterion/criterion.h>

#include "mos6502/mos6502.h"
#include "mos6502/enums.h"
#include "mos6502/tests.h"

TestSuite(mos6502_stat, .init = setup, .fini = teardown);

Test(mos6502_stat, clc)
{
    cpu->P = MOS_CARRY | MOS_DECIMAL;
    mos6502_handle_clc(cpu, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);
}

Test(mos6502_stat, cld)
{
    cpu->P = MOS_DECIMAL | MOS_CARRY;
    mos6502_handle_cld(cpu, 0);
    cr_assert_eq(cpu->P & MOS_DECIMAL, 0);
}

Test(mos6502_stat, cli)
{
    cpu->P = MOS_CARRY | MOS_INTERRUPT;
    mos6502_handle_cli(cpu, 0);
    cr_assert_eq(cpu->P & MOS_INTERRUPT, 0);
}

Test(mos6502_stat, clv)
{
    cpu->P = MOS_CARRY | MOS_OVERFLOW;
    mos6502_handle_clv(cpu, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
}

Test(mos6502_stat, sec)
{
    cpu->P = 0;
    mos6502_handle_sec(cpu, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
}

Test(mos6502_stat, sed)
{
    cpu->P = 0;
    mos6502_handle_sed(cpu, 0);
    cr_assert_eq(cpu->P & MOS_DECIMAL, MOS_DECIMAL);
}

Test(mos6502_stat, sei)
{
    cpu->P = 0;
    mos6502_handle_sei(cpu, 0);
    cr_assert_eq(cpu->P & MOS_INTERRUPT, MOS_INTERRUPT);
}
