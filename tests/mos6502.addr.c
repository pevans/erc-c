#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_addr, .init = setup, .fini = teardown);

Test(mos6502_addr, mode)
{
    cr_assert_eq(mos6502_addr_mode(0xEA), IMP);
    cr_assert_eq(mos6502_addr_mode(0xD6), ZPX);
    cr_assert_eq(mos6502_addr_mode(0xF0), REL);
}

Test(mos6502_addr, addr_mode_acc)
{
    cpu->A = 123;
    cr_assert_eq(mos6502_resolve_acc(cpu), 123);
}

Test(mos6502_addr, addr_mode_abs)
{
    mos6502_set(cpu, 0x1234, 111);
    SET_PC_BYTE(cpu, 1, 0x34);
    SET_PC_BYTE(cpu, 2, 0x12);
    cr_assert_eq(mos6502_resolve_abs(cpu), 111);
}

Test(mos6502_addr, addr_mode_abx_carry0)
{
    mos6502_set(cpu, 0x1234, 111);
    SET_PC_BYTE(cpu, 1, 0x30);
    SET_PC_BYTE(cpu, 2, 0x12);
    cpu->X = 4;
    cpu->P = cpu->P & ~MOS_CARRY;
    cr_assert_eq(mos6502_resolve_abx(cpu), 111);
}

Test(mos6502_addr, addr_mode_abx_carry1)
{
    mos6502_set(cpu, 0x1234, 111);
    SET_PC_BYTE(cpu, 1, 0x30);
    SET_PC_BYTE(cpu, 2, 0x12);
    cpu->X = 3;
    cr_assert_eq(mos6502_resolve_abx(cpu), 111);
}

Test(mos6502_addr, addr_mode_aby_carry0)
{
    mos6502_set(cpu, 0x1234, 111);
    SET_PC_BYTE(cpu, 1, 0x30);
    SET_PC_BYTE(cpu, 2, 0x12);
    cpu->Y = 4;
    cpu->P = cpu->P & ~MOS_CARRY;
    cr_assert_eq(mos6502_resolve_aby(cpu), 111);
}

Test(mos6502_addr, addr_mode_aby_carry1)
{
    mos6502_set(cpu, 0x1234, 111);
    SET_PC_BYTE(cpu, 1, 0x30);
    SET_PC_BYTE(cpu, 2, 0x12);
    cpu->Y = 3;
    cr_assert_eq(mos6502_resolve_aby(cpu), 111);
}

Test(mos6502_addr, addr_mode_imm)
{
    SET_PC_BYTE(cpu, 1, 0x12);
    cr_assert_eq(mos6502_resolve_imm(cpu), 0x12);
}

Test(mos6502_addr, addr_mode_idx)
{
    mos6502_set(cpu, 0x17, 0x23);
    mos6502_set(cpu, 0x23, 123);

    SET_PC_BYTE(cpu, 1, 0x12);
    cpu->X = 5;
    cr_assert_eq(mos6502_resolve_idx(cpu), 123);
}

Test(mos6502_addr, addr_mode_idy)
{
    mos6502_set(cpu, 0x12, 0x23);
    mos6502_set(cpu, 0x28, 123);

    SET_PC_BYTE(cpu, 1, 0x12);
    cpu->Y = 5;
    cr_assert_eq(mos6502_resolve_idy(cpu), 123);
}

Test(mos6502_addr, addr_mode_ind)
{
    mos6502_set(cpu, 0x1234, 0x45);
    mos6502_set(cpu, 0x1235, 0x23);
    mos6502_set(cpu, 0x2345, 123);

    SET_PC_BYTE(cpu, 1, 0x34);
    SET_PC_BYTE(cpu, 2, 0x12);
    cr_assert_eq(mos6502_resolve_ind(cpu), 123);
}

Test(mos6502_addr, addr_mode_rel_positive)
{
    cpu->PC = 123;
    SET_PC_BYTE(cpu, 1, 88);
    cr_assert_eq(mos6502_resolve_rel(cpu), 0);
    cr_assert_eq(cpu->eff_addr, 213);
}

Test(mos6502_addr, addr_mode_rel_negative)
{
    cpu->PC = 123;
    SET_PC_BYTE(cpu, 1, 216);
    cr_assert_eq(mos6502_resolve_rel(cpu), 0);
    cr_assert_eq(cpu->eff_addr, 85);
}

Test(mos6502_addr, addr_mode_zpg)
{
    mos6502_set(cpu, 0x0034, 222);
    SET_PC_BYTE(cpu, 1, 0x34);
    cr_assert_eq(mos6502_resolve_zpg(cpu), 222);
}

Test(mos6502_addr, addr_mode_zpx)
{
    mos6502_set(cpu, 0x0034, 222);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->X = 4;
    cr_assert_eq(mos6502_resolve_zpx(cpu), 222);
}

Test(mos6502_addr, addr_mode_zpy)
{
    mos6502_set(cpu, 0x0034, 222);
    SET_PC_BYTE(cpu, 1, 0x2F);
    cpu->Y = 5;
    cr_assert_eq(mos6502_resolve_zpy(cpu), 222);
}

