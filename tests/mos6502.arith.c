#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, adc)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_adc(cpu, 3);
    cr_assert_eq(cpu->A, 8);

    cpu->P |= CARRY;
    mos6502_handle_adc(cpu, 64);
    cr_assert_eq(cpu->A, 73);

    END_CPU_TEST(mos6502);
}

Test(mos6502, cmp)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_cmp(cpu, 3);
    cr_assert_eq(cpu->P & CARRY, CARRY);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & ZERO, 0);

    cpu->A = 3;
    mos6502_handle_cmp(cpu, 3);
    cr_assert_eq(cpu->P & CARRY, 0);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & ZERO, ZERO);

    cpu->A = 192;
    mos6502_handle_cmp(cpu, 3);
    cr_assert_eq(cpu->P & CARRY, CARRY);
    cr_assert_eq(cpu->P & NEGATIVE, NEGATIVE);
    cr_assert_eq(cpu->P & ZERO, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, cpx)
{
    START_CPU_TEST(mos6502);

    cpu->X = 5;
    mos6502_handle_cpx(cpu, 3);
    cr_assert_eq(cpu->P & CARRY, CARRY);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & ZERO, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, cpy)
{
    START_CPU_TEST(mos6502);

    cpu->Y = 5;
    mos6502_handle_cpy(cpu, 3);
    cr_assert_eq(cpu->P & CARRY, CARRY);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & ZERO, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, dec)
{
    START_CPU_TEST(mos6502);

    // Note that DEC does NOT decrement the accumulator if the last
    // address is not set. It does _nothing_.
    cpu->A = 5;
    mos6502_handle_dec(cpu, 0);
    cr_assert_neq(cpu->A, 4);

    cpu->last_addr = 123;
    vm_segment_set(cpu->memory, 123, 44);

    // Note _also_ that DEC expects the number to be decremented will be
    // passed in as the effective operand, although it doesn't
    // necessarily need for that to be so.
    mos6502_handle_dec(cpu, 44);
    cr_assert_eq(vm_segment_get(cpu->memory, 123), 43);

    END_CPU_TEST(mos6502);
}

Test(mos6502, dex)
{
    START_CPU_TEST(mos6502);

    cpu->X = 5;
    mos6502_handle_dex(cpu, 0);
    cr_assert_eq(cpu->X, 4);

    END_CPU_TEST(mos6502);
}

Test(mos6502, dey)
{
    START_CPU_TEST(mos6502);

    cpu->Y = 5;
    mos6502_handle_dey(cpu, 0);
    cr_assert_eq(cpu->Y, 4);

    END_CPU_TEST(mos6502);
}

Test(mos6502, inc)
{
    START_CPU_TEST(mos6502);

    cpu->last_addr = 123;
    mos6502_handle_inc(cpu, 55);
    cr_assert_eq(vm_segment_get(cpu->memory, 123), 56);

    END_CPU_TEST(mos6502);
}

Test(mos6502, inx)
{
    START_CPU_TEST(mos6502);

    cpu->X = 5;
    mos6502_handle_inx(cpu, 0);
    cr_assert_eq(cpu->X, 6);

    END_CPU_TEST(mos6502);
}

Test(mos6502, iny)
{
    START_CPU_TEST(mos6502);

    cpu->Y = 5;
    mos6502_handle_iny(cpu, 0);
    cr_assert_eq(cpu->Y, 6);

    END_CPU_TEST(mos6502);
}

Test(mos6502, sbc)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_sbc(cpu, 3);
    cr_assert_eq(cpu->A, 2);

    cpu->P |= CARRY;
    cpu->A = 16;
    mos6502_handle_sbc(cpu, 8);
    cr_assert_eq(cpu->A, 7);

    END_CPU_TEST(mos6502);
}
