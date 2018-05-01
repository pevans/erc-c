#include <criterion/criterion.h>

#include "mos6502/mos6502.h"
#include "mos6502/enums.h"
#include "mos6502/tests.h"

TestSuite(mos6502_arith, .init = setup, .fini = teardown);

Test(mos6502_arith, adc)
{
    cpu->A = 5;
    mos6502_handle_adc(cpu, 3);
    cr_assert_eq(cpu->A, 9);

    cpu->A = 0xfe;
    mos6502_handle_adc(cpu, 0x5);
    cr_assert_eq(cpu->A, (vm_8bit)(0xfe + 0x5));

    cpu->P &= ~MOS_CARRY;
    cpu->A = 9;
    mos6502_handle_adc(cpu, 64);
    cr_assert_eq(cpu->A, 73);

    // This should handle decimal mode without complaint
    cpu->P |= MOS_DECIMAL;
    cpu->A = 0x18;
    mos6502_handle_adc(cpu, 0x3);
    cr_assert_eq(cpu->A, 0x21);
}

Test(mos6502_arith, adc_dec)
{
    cpu->P &= ~MOS_CARRY;
    cpu->A = 0x05;
    mos6502_handle_adc_dec(cpu, 0x10);
    cr_assert_eq(cpu->A, 0x15);

    cpu->A = 0x98;
    mos6502_handle_adc_dec(cpu, 0x3);
    cr_assert_eq(cpu->A, 0x1);

    // Test that A + M + 1 works for carry
    cpu->P |= MOS_CARRY;
    cpu->A = 0x15;
    mos6502_handle_adc_dec(cpu, 0x13);
    cr_assert_eq(cpu->A, 0x29);
}

/*
 * In CMP, the following occurs:
 *
 * Z = 1 if A = DATA (which is to say: Z = 1 if A - DATA = 0)
 * N = 1 if DATA BIT 7 is high
 * C = 1 if A >= DATA
 *
 * Only status flags are changed by the operation of CMP.
 *
 * We need to test four permutations:
 *   - A > DATA
 *   - A = DATA
 *   - A < DATA
 */
Test(mos6502_arith, cmp)
{
    cpu->A = 123;
    mos6502_handle_cmp(cpu, cpu->A - 1);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);

    mos6502_handle_cmp(cpu, cpu->A);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);

    mos6502_handle_cmp(cpu, cpu->A + 1);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
}

/*
 * As in the CMP test, we check the same qualities: X > DATA, X = DATA,
 * X < DATA.
 */
Test(mos6502_arith, cpx)
{
    cpu->X = 123;
    mos6502_handle_cpx(cpu, cpu->X - 1);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);

    mos6502_handle_cpx(cpu, cpu->X);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);

    mos6502_handle_cpx(cpu, cpu->X + 1);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
}

/*
 * And, finally, we check similarly as to the CPX test, except with Y
 * instead of X.
 */
Test(mos6502_arith, cpy)
{
    cpu->Y = 123;
    mos6502_handle_cpy(cpu, cpu->Y - 1);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);

    mos6502_handle_cpy(cpu, cpu->Y);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);

    mos6502_handle_cpy(cpu, cpu->Y + 1);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
}

Test(mos6502_arith, dec)
{
    cpu->A = 5;
    cpu->addr_mode = ACC;
    mos6502_handle_dec(cpu, 0);
    cr_assert_eq(cpu->A, 4);

    cpu->eff_addr = 0;
    cpu->A = 0;
    mos6502_handle_dec(cpu, 0);
    cr_assert_eq(cpu->A, 0xff);

    cpu->eff_addr = 123;
    mos6502_set(cpu, 123, 44);

    cpu->addr_mode = 0;
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

    cpu->A = 8;
    cpu->eff_addr = 0;
    cpu->addr_mode = ACC;
    mos6502_handle_inc(cpu, 0);
    cr_assert_eq(cpu->A, 9);

    cpu->A = 0xff;
    mos6502_handle_inc(cpu, 0);
    cr_assert_eq(cpu->A, 0);
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

    cpu->A = 0x3;
    mos6502_handle_sbc(cpu, 5);
    cr_assert_eq(cpu->A, (vm_8bit)(0x3 - 0x5));

    cpu->P &= ~MOS_CARRY;
    cpu->A = 16;
    mos6502_handle_sbc(cpu, 8);
    cr_assert_eq(cpu->A, 7);

    cpu->P |= MOS_DECIMAL;
    cpu->A = 0x12;
    mos6502_handle_sbc(cpu, 0x3);
    cr_assert_eq(cpu->A, 0x9);
}

Test(mos6502_arith, sbc_dec)
{
    cpu->P = 0;
    cpu->A = 0x15;
    mos6502_handle_sbc_dec(cpu, 0x6);
    cr_assert_eq(cpu->A, 0x8);

    cpu->P |= MOS_CARRY;
    cpu->A = 0x12;
    mos6502_handle_sbc_dec(cpu, 0x2);
    cr_assert_eq(cpu->A, 0x10);

    cpu->A = 0x2;
    mos6502_handle_sbc_dec(cpu, 0x3);
    cr_assert_eq(cpu->A, 0x99);
}
