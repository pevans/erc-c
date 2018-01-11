#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_bits, .init = setup, .fini = teardown);

Test(mos6502_bits, and)
{
    cpu->A = 5;
    mos6502_handle_and(cpu, 1);
    cr_assert_eq(cpu->A, 1);

    cpu->A = 5;
    mos6502_handle_and(cpu, 4);
    cr_assert_eq(cpu->A, 4);
}

Test(mos6502_bits, asl)
{
    mos6502_handle_asl(cpu, 5);
    cr_assert_eq(cpu->A, 10);

    cpu->last_addr = 123;
    mos6502_handle_asl(cpu, 22);
    cr_assert_eq(mos6502_get(cpu, 123), 44);
}

Test(mos6502_bits, bit)
{
    cpu->A = 5;
    mos6502_handle_bit(cpu, 129);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_bit(cpu, 193);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_bit(cpu, 65);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_bit(cpu, 33);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);

    mos6502_handle_bit(cpu, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, 0);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
}

Test(mos6502_bits, eor)
{
    cpu->A = 5;
    mos6502_handle_eor(cpu, 4);
    cr_assert_eq(cpu->A, 1);

    cpu->A = 5;
    mos6502_handle_eor(cpu, 1);
    cr_assert_eq(cpu->A, 4);
}

Test(mos6502_bits, lsr)
{
    mos6502_handle_lsr(cpu, 5);
    cr_assert_eq(cpu->A, 2);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);

    cpu->last_addr = 123;
    mos6502_handle_lsr(cpu, 22);
    cr_assert_eq(mos6502_get(cpu, 123), 11);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
}

Test(mos6502_bits, ora)
{
    cpu->A = 5;
    mos6502_handle_ora(cpu, 4);
    cr_assert_eq(cpu->A, 5);

    cpu->A = 5;
    mos6502_handle_ora(cpu, 10);
    cr_assert_eq(cpu->A, 15);
}

Test(mos6502_bits, rol)
{
    mos6502_handle_rol(cpu, 8);
    cr_assert_eq(cpu->A, 16);

    cpu->last_addr = 234;
    mos6502_handle_rol(cpu, 128);
    cr_assert_eq(mos6502_get(cpu, 234), 1);
}

Test(mos6502_bits, ror)
{
    mos6502_handle_ror(cpu, 64);
    cr_assert_eq(cpu->A, 32);

    cpu->last_addr = 123;
    mos6502_handle_ror(cpu, 1);
    cr_assert_eq(mos6502_get(cpu, 123), 128);
}
