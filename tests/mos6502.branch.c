#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_branch, .init = setup, .fini = teardown);

Test(mos6502_branch, bcc)
{
    cpu->last_addr = 123;
    mos6502_handle_bcc(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= MOS_CARRY;
    cpu->last_addr = 125;
    mos6502_handle_bcc(cpu, 0);
    cr_assert_neq(cpu->PC, 127);
}

Test(mos6502_branch, bcs)
{
    cpu->last_addr = 123;
    mos6502_handle_bcs(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= MOS_CARRY;
    cpu->last_addr = 125;
    mos6502_handle_bcs(cpu, 0);
    cr_assert_eq(cpu->PC, 125);
}

Test(mos6502_branch, beq)
{
    cpu->last_addr = 123;
    mos6502_handle_beq(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= MOS_ZERO;
    cpu->last_addr = 125;
    mos6502_handle_beq(cpu, 0);
    cr_assert_eq(cpu->PC, 125);
}

Test(mos6502_branch, bmi)
{
    cpu->last_addr = 123;
    mos6502_handle_bmi(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= MOS_NEGATIVE;
    cpu->last_addr = 125;
    mos6502_handle_bmi(cpu, 0);
    cr_assert_eq(cpu->PC, 125);
}

Test(mos6502_branch, bne)
{
    cpu->last_addr = 123;
    mos6502_handle_bne(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= MOS_ZERO;
    cpu->last_addr = 125;
    mos6502_handle_bne(cpu, 0);
    cr_assert_neq(cpu->PC, 127);
}

Test(mos6502_branch, bpl)
{
    cpu->last_addr = 123;
    mos6502_handle_bpl(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= MOS_NEGATIVE;
    cpu->last_addr = 125;
    mos6502_handle_bpl(cpu, 0);
    cr_assert_neq(cpu->PC, 127);
}

Test(mos6502_branch, bvc)
{
    cpu->last_addr = 123;
    mos6502_handle_bvc(cpu, 0);
    cr_assert_eq(cpu->PC, 123);

    cpu->P |= MOS_OVERFLOW;
    cpu->last_addr = 125;
    mos6502_handle_bvc(cpu, 0);
    cr_assert_neq(cpu->PC, 127);
}

Test(mos6502_branch, bvs)
{
    cpu->last_addr = 123;
    mos6502_handle_bvs(cpu, 0);
    cr_assert_neq(cpu->PC, 123);

    cpu->P |= MOS_OVERFLOW;
    cpu->last_addr = 125;
    mos6502_handle_bvs(cpu, 0);
    cr_assert_eq(cpu->PC, 125);
}
