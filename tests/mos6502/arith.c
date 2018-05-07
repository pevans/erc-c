#include <criterion/criterion.h>

#include "mos6502/mos6502.h"
#include "mos6502/enums.h"
#include "mos6502/tests.h"

TestSuite(mos6502_arith, .init = setup, .fini = teardown);

/*
 * ADC is a deceptively simple instruction. It works like this:
 *
 * A = A + DATA + C
 *
 * If D = 1, then ADC works in an entirely different manner, treating
 * the input as BCD (Binary-Coded Decimal).
 *
 * Z = 1 if RESULT = 0
 * N = 1 if RESULT has BIT 7 high
 * V = 1 if (A ^ DATA) has BIT 7 high AND (A ^ RESULT) has BIT 7 high
 * C = 1 if (16-BIT) RESULT > 0xFF
 */
Test(mos6502_arith, adc)
{
    vm_8bit start = 30,
            main = 60,
            ztest = 0x100 - start,
            vtest = 0x80,
            ctest = 0xff;

    // Test with and without carry
    cpu->P |= MOS_CARRY;
    cpu->A = start;
    mos6502_handle_adc(cpu, main);
    cr_assert_eq(cpu->A, start + main + 1);
    cpu->P &= ~MOS_CARRY;
    cpu->A = start;
    mos6502_handle_adc(cpu, main);
    cr_assert_eq(cpu->A, start + main);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);

    // If an add results in Z = 1, it necessarily implies C = 1. Say you
    // have A = 1, and add -1. With two's complement, the binary coded
    // form of -1 is 0xff; thus 0xff + 0x1 = 0x100, or 0x00 after
    // variable overflow, and thus C = 1.
    //
    // NOTE: variable overflow is e.g. when you go from 0xff to 0x00; it
    // means something different from the V/OVERFLOW status in the 6502
    // chip, which cares about going from a positive to a negative, or
    // from a negative to a positive.
    cpu->A = start;
    cpu->P &= ~MOS_CARRY;
    mos6502_handle_adc(cpu, ztest);
    cr_assert_eq(cpu->A, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);

    // We can test both negative and overflow here. We could do a
    // separate test on the N flag if we set A = 0x80 and added a small
    // number, like 0x3; we would essentially begin with a negative
    // number and end with a negative number.
    cpu->A = start;
    cpu->P &= ~MOS_CARRY;
    mos6502_handle_adc(cpu, vtest);
    cr_assert_eq(cpu->A, start + vtest);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);

    cpu->A = start;
    cpu->P &= ~MOS_CARRY;
    mos6502_handle_adc(cpu, ctest);
    // Cast to vm_8bit since we're working with variable overflow
    cr_assert_eq(cpu->A, (vm_8bit)(start + ctest));
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);

    // This should handle decimal mode without complaint
    cpu->P |= MOS_DECIMAL;
    cpu->P &= ~MOS_CARRY;
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

/*
 * In DEC, we simply decrement an address of memory (or the accumulator)
 * by 1. 
 *
 * Status flags:
 *   - N = 1 if (DATA - 1) has BIT 7 high
 *   - Z = 1 if (DATA - 1) = 0, i.e. if DATA = 1
 */
Test(mos6502_arith, dec)
{
    vm_8bit main = 123,
            ntest = 0,
            ztest = 1;

    vm_16bit addr = 0x123;

    cpu->A = main;
    cpu->addr_mode = ACC;
    mos6502_handle_dec(cpu, cpu->A);
    cr_assert_eq(cpu->A, main - 1);

    cpu->eff_addr = addr;
    cpu->addr_mode = ABS;
    mos6502_handle_dec(cpu, main);
    cr_assert_eq(mos6502_get(cpu, addr), main - 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_dec(cpu, ntest);
    // Cast ntest - 1 so that the result we compare is 8-bit negative
    // and not 32-bit negative
    cr_assert_eq(mos6502_get(cpu, addr), (vm_8bit)(ntest - 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_dec(cpu, ztest);
    cr_assert_eq(mos6502_get(cpu, addr), ztest - 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

/*
 * Same principles as the test for DEC, but there is no need to test for
 * memory sets; DEX only modifies the X register.
 */
Test(mos6502_arith, dex)
{
    vm_8bit main = 123,
            ntest = 0,
            ztest = 1;

    cpu->X = main;
    cpu->addr_mode = ACC;

    mos6502_handle_dex(cpu, cpu->X);
    cr_assert_eq(cpu->X, main - 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_dex(cpu, ntest);
    // Cast ntest - 1 so that the result we compare is 8-bit negative
    // and not 32-bit negative
    cr_assert_eq(cpu->X, (vm_8bit)(ntest - 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_dex(cpu, ztest);
    cr_assert_eq(cpu->X, ztest - 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

/*
 * Similar tests as for DEX, except for the Y register instead of the X.
 */
Test(mos6502_arith, dey)
{
    vm_8bit main = 123,
            ntest = 0,
            ztest = 1;

    cpu->Y = main;
    cpu->addr_mode = ACC;

    mos6502_handle_dey(cpu, cpu->Y);
    cr_assert_eq(cpu->Y, main - 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_dey(cpu, ntest);
    // Cast ntest - 1 so that the result we compare is 8-bit negative
    // and not 32-bit negative
    cr_assert_eq(cpu->Y, (vm_8bit)(ntest - 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_dey(cpu, ztest);
    cr_assert_eq(cpu->Y, ztest - 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

/*
 * The INC instruction works similarly to DEC; the same status flags are
 * updated. But, of course, we increment rather than decrement.
 */
Test(mos6502_arith, inc)
{
    vm_8bit main = 123,
            ntest = 0x7F,
            ztest = 0xFF;

    vm_16bit addr = 0x123;

    cpu->A = main;
    cpu->addr_mode = ACC;
    mos6502_handle_inc(cpu, main);
    cr_assert_eq(cpu->A, main + 1);

    cpu->eff_addr = addr;
    cpu->addr_mode = ABS;
    mos6502_handle_inc(cpu, main);
    cr_assert_eq(mos6502_get(cpu, addr), main + 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_inc(cpu, ntest);
    // Cast ntest - 1 so that the result we compare is 8-bit negative
    // and not 32-bit negative
    cr_assert_eq(mos6502_get(cpu, addr), (vm_8bit)(ntest + 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_inc(cpu, ztest);
    cr_assert_eq(mos6502_get(cpu, addr), (vm_8bit)(ztest + 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

Test(mos6502_arith, inx)
{
    vm_8bit main = 123,
            ntest = 0x7F,
            ztest = 0xFF;

    cpu->addr_mode = ACC;
    mos6502_handle_inx(cpu, main);
    cr_assert_eq(cpu->X, main + 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_inx(cpu, ntest);
    // Cast ntest - 1 so that the result we compare is 8-bit negative
    // and not 32-bit negative
    cr_assert_eq(cpu->X, (vm_8bit)(ntest + 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_inx(cpu, ztest);
    cr_assert_eq(cpu->X, (vm_8bit)(ztest + 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

Test(mos6502_arith, iny)
{
    vm_8bit main = 123,
            ntest = 0x7F,
            ztest = 0xFF;

    cpu->addr_mode = ACC;
    mos6502_handle_iny(cpu, main);
    cr_assert_eq(cpu->Y, main + 1);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_iny(cpu, ntest);
    // Cast ntest - 1 so that the result we compare is 8-bit negative
    // and not 32-bit negative
    cr_assert_eq(cpu->Y, (vm_8bit)(ntest + 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_iny(cpu, ztest);
    cr_assert_eq(cpu->Y, (vm_8bit)(ztest + 1));
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

/*
 * SBC has a lot of similarities with the spirit of ADC; it is
 * deceptively simple, doing the following:
 *
 * A = A - DATA if C = 1
 * A = A - DATA - 1 if C = 0
 *
 * Z = 1 if RESULT = 0
 * V = 1 if (A ^ DATA) has BIT 7 high AND (A ^ RESULT) has BIT 7 high
 * N = 1 if RESULT has BIT 7 high
 * C = 1 if (16-BIT) RESULT < 0
 */
Test(mos6502_arith, sbc)
{
    vm_8bit start = 90,
            main = 60,
            ztest = start,
            vtest = 0x80,
            ntest = 0xff;

    // Test with and without carry
    cpu->P |= MOS_CARRY;
    cpu->A = start;
    mos6502_handle_sbc(cpu, main);
    cr_assert_eq(cpu->A, start - main);

    cpu->P &= ~MOS_CARRY;
    cpu->A = start;
    mos6502_handle_sbc(cpu, main);
    cr_assert_eq(cpu->A, start - main - 1);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);

    cpu->A = start;
    cpu->P |= MOS_CARRY;
    mos6502_handle_sbc(cpu, ztest);
    cr_assert_eq(cpu->A, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);

    cpu->A = start;
    cpu->P |= MOS_CARRY;
    mos6502_handle_sbc(cpu, vtest);
    cr_assert_eq(cpu->A, (vm_8bit)(start - vtest));
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);

    cpu->A = start;
    cpu->P |= MOS_CARRY;
    mos6502_handle_sbc(cpu, ntest);
    cr_assert_eq(cpu->A, (vm_8bit)(start - ntest));
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_CARRY, 0);
    
    // FIXME: add a decimal test
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
