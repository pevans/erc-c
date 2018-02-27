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

    // Test if carry is set properly
    cpu->P &= ~MOS_CARRY;
    mos6502_handle_asl(cpu, 150);
    cr_assert_eq(cpu->A, 44);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);

    cpu->eff_addr = 123;
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

Test(mos6502_bits, bim)
{
    // This version of BIT should not modify the NV flags
    cpu->P |= MOS_NEGATIVE;
    cpu->P |= MOS_OVERFLOW;

    cpu->A = 63;
    mos6502_handle_bim(cpu, 123);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);

    cpu->A = 4;
    mos6502_handle_bim(cpu, 123);
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

    cpu->eff_addr = 123;
    mos6502_handle_lsr(cpu, 11);
    cr_assert_eq(mos6502_get(cpu, 123), 5);
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
    cr_assert_eq(cpu->A, 17);

    cpu->eff_addr = 234;
    mos6502_handle_rol(cpu, 128);
    cr_assert_eq(mos6502_get(cpu, 234), 0);
}

Test(mos6502_bits, ror)
{
    mos6502_handle_ror(cpu, 64);
    cr_assert_eq(cpu->A, 160);

    cpu->eff_addr = 123;
    mos6502_handle_ror(cpu, 1);
    mos6502_handle_ror(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, 123), 128);
}

Test(mos6502_bits, trb)
{
    cpu->A = 6;
    mos6502_handle_trb(cpu, 3);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cpu->A = 9;
    mos6502_handle_trb(cpu, 2);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);

    cpu->eff_addr = 111;
    mos6502_set(cpu, cpu->eff_addr, 123);
    mos6502_handle_trb(cpu, 123);

    cr_assert_eq(mos6502_get(cpu, cpu->eff_addr),
                 (cpu->A ^ 0xff) & 123);
}

Test(mos6502_bits, tsb)
{
    // I borrowed this code from the trb test; TSB and TRB do exactly
    // the same thing in regards to the zero bit.
    cpu->A = 6;
    mos6502_handle_trb(cpu, 3);
    cr_assert_eq(cpu->P & MOS_ZERO, 0);
    cpu->A = 9;
    mos6502_handle_trb(cpu, 2);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);

    // This is similar to the segment in the trb test that focuses on
    // the resetting (clearing) of bits, but modified to account for the
    // differing result that tsb would provide--namely that the 1s in A
    // will be set in the location in memory, rather than cleared.
    cpu->eff_addr = 111;
    mos6502_set(cpu, cpu->eff_addr, 123);
    mos6502_handle_tsb(cpu, 123);

    cr_assert_eq(mos6502_get(cpu, cpu->eff_addr), cpu->A | 123);
}

