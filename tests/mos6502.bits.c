#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, and)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_and(cpu, 1);
    cr_assert_eq(cpu->A, 1);

    cpu->A = 5;
    mos6502_handle_and(cpu, 4);
    cr_assert_eq(cpu->A, 4);

    END_CPU_TEST(mos6502);
}

Test(mos6502, asl)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_asl(cpu, 5);
    cr_assert_eq(cpu->A, 10);

    cpu->last_addr = 123;
    mos6502_handle_asl(cpu, 22);
    cr_assert_eq(vm_segment_get(cpu->memory, 123), 44);

    END_CPU_TEST(mos6502);
}

Test(mos6502, bit)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_bit(cpu, 129);
    cr_assert_eq(cpu->P & NEGATIVE, NEGATIVE);
    cr_assert_eq(cpu->P & OVERFLOW, 0);
    cr_assert_eq(cpu->P & ZERO, 0);

    mos6502_handle_bit(cpu, 193);
    cr_assert_eq(cpu->P & NEGATIVE, NEGATIVE);
    cr_assert_eq(cpu->P & OVERFLOW, OVERFLOW);
    cr_assert_eq(cpu->P & ZERO, 0);

    mos6502_handle_bit(cpu, 65);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & OVERFLOW, OVERFLOW);
    cr_assert_eq(cpu->P & ZERO, 0);

    mos6502_handle_bit(cpu, 33);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & OVERFLOW, 0);
    cr_assert_eq(cpu->P & ZERO, 0);

    mos6502_handle_bit(cpu, 0);
    cr_assert_eq(cpu->P & NEGATIVE, 0);
    cr_assert_eq(cpu->P & OVERFLOW, 0);
    cr_assert_eq(cpu->P & ZERO, ZERO);

    END_CPU_TEST(mos6502);
}

Test(mos6502, eor)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_eor(cpu, 4);
    cr_assert_eq(cpu->A, 1);

    cpu->A = 5;
    mos6502_handle_eor(cpu, 1);
    cr_assert_eq(cpu->A, 4);

    END_CPU_TEST(mos6502);
}

Test(mos6502, lsr)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_lsr(cpu, 5);
    cr_assert_eq(cpu->A, 2);
    cr_assert_eq(cpu->P & CARRY, CARRY);

    cpu->last_addr = 123;
    mos6502_handle_lsr(cpu, 22);
    cr_assert_eq(vm_segment_get(cpu->memory, 123), 11);
    cr_assert_eq(cpu->P & CARRY, 0);

    END_CPU_TEST(mos6502);
}

Test(mos6502, ora)
{
    START_CPU_TEST(mos6502);

    cpu->A = 5;
    mos6502_handle_ora(cpu, 4);
    cr_assert_eq(cpu->A, 5);

    cpu->A = 5;
    mos6502_handle_ora(cpu, 10);
    cr_assert_eq(cpu->A, 15);

    END_CPU_TEST(mos6502);
}

Test(mos6502, rol)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_rol(cpu, 8);
    cr_assert_eq(cpu->A, 16);

    cpu->last_addr = 234;
    mos6502_handle_rol(cpu, 128);
    cr_assert_eq(vm_segment_get(cpu->memory, 234), 1);

    END_CPU_TEST(mos6502);
}

Test(mos6502, ror)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_ror(cpu, 64);
    cr_assert_eq(cpu->A, 32);

    cpu->last_addr = 123;
    mos6502_handle_ror(cpu, 1);
    cr_assert_eq(vm_segment_get(cpu->memory, 123), 128);

    END_CPU_TEST(mos6502);
}
