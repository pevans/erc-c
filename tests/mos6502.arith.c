#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_arith, .init = setup, .fini = teardown);

Test(mos6502_arith, adc)
{
    cpu->A = 5;
    mos6502_handle_adc(cpu, 3);
    cr_assert_eq(cpu->A, 8);

    cpu->P |= MOS_CARRY;
    mos6502_handle_adc(cpu, 64);
    cr_assert_eq(cpu->A, 73);
}

Test(mos6502_arith, cmp)
{
    cpu->A = 5;
    mos6502_handle_cmp(cpu, 3);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    cpu->A = 3;
    mos6502_handle_cmp(cpu, 3);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);

    cpu->A = 192;
    mos6502_handle_cmp(cpu, 3);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
}

Test(mos6502_arith, cpx)
{
    cpu->X = 5;
    mos6502_handle_cpx(cpu, 3);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
}

Test(mos6502_arith, cpy)
{
    cpu->Y = 5;
    mos6502_handle_cpy(cpu, 3);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
}

Test(mos6502_arith, dec)
{
    // Note that DEC does NOT decrement the accumulator if the last
    // address is not set. It does _nothing_.
    cpu->A = 5;
    mos6502_handle_dec(cpu, 0);
    cr_assert_neq(cpu->A, 4);

    cpu->eff_addr = 123;
    mos6502_set(cpu, 123, 44);

    // Note _also_ that DEC expects the number to be decremented will be
    // passed in as the effective operand, although it doesn't
    // necessarily need for that to be so.
    mos6502_handle_dec(cpu, 44);
    cr_assert_eq(mos6502_get(cpu, 123), 43);
}

Test(mos6502_arith, dex)
{
    cpu->X = 5;
    mos6502_handle_dex(cpu, 0);
    cr_assert_eq(cpu->X, 4);
}

Test(mos6502_arith, dey)
{
    cpu->Y = 5;
    mos6502_handle_dey(cpu, 0);
    cr_assert_eq(cpu->Y, 4);
}

Test(mos6502_arith, inc)
{
    cpu->eff_addr = 123;
    mos6502_handle_inc(cpu, 55);
    cr_assert_eq(mos6502_get(cpu, 123), 56);
}

Test(mos6502_arith, inx)
{
    cpu->X = 5;
    mos6502_handle_inx(cpu, 0);
    cr_assert_eq(cpu->X, 6);
}

Test(mos6502_arith, iny)
{
    cpu->Y = 5;
    mos6502_handle_iny(cpu, 0);
    cr_assert_eq(cpu->Y, 6);
}

Test(mos6502_arith, sbc)
{
    cpu->A = 5;
    mos6502_handle_sbc(cpu, 3);
    cr_assert_eq(cpu->A, 2);

    cpu->P |= MOS_CARRY;
    cpu->A = 16;
    mos6502_handle_sbc(cpu, 8);
    cr_assert_eq(cpu->A, 7);
}
