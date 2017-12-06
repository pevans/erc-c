#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_addr, .init = setup, .fini = teardown);

Test(mos6502_addr, addr_mode)
{
    cr_assert_eq(mos6502_addr_mode(0xEA), IMP);
    cr_assert_eq(mos6502_addr_mode(0xD6), ZPX);
    cr_assert_eq(mos6502_addr_mode(0xF0), REL);
}

Test(mos6502_addr, get_address_resolver)
{
    cr_assert_eq(mos6502_get_address_resolver(0x0A), mos6502_resolve_acc);
    cr_assert_eq(mos6502_get_address_resolver(0x20), mos6502_resolve_abs);
    cr_assert_eq(mos6502_get_address_resolver(0xBC), mos6502_resolve_abx);
    cr_assert_eq(mos6502_get_address_resolver(0x19), mos6502_resolve_aby);
    cr_assert_eq(mos6502_get_address_resolver(0xA0), mos6502_resolve_imm);
    cr_assert_eq(mos6502_get_address_resolver(0x6C), mos6502_resolve_ind);
    cr_assert_eq(mos6502_get_address_resolver(0x01), mos6502_resolve_idx);
    cr_assert_eq(mos6502_get_address_resolver(0x11), mos6502_resolve_idy);
    cr_assert_eq(mos6502_get_address_resolver(0x10), mos6502_resolve_rel);
    cr_assert_eq(mos6502_get_address_resolver(0x05), mos6502_resolve_zpg);
    cr_assert_eq(mos6502_get_address_resolver(0x15), mos6502_resolve_zpx);
    cr_assert_eq(mos6502_get_address_resolver(0x96), mos6502_resolve_zpy);
}

Test(mos6502_addr, addr_mode_acc)
{
    cpu->A = 123;
    cr_assert_eq(mos6502_resolve_acc(cpu), 123);
}

Test(mos6502_addr, addr_mode_abs)
{
    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x34);
    cr_assert_eq(mos6502_resolve_abs(cpu), 111);
}

Test(mos6502_addr, addr_mode_abx_carry0)
{
    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->X = 4;
    cr_assert_eq(mos6502_resolve_abx(cpu), 111);
}

Test(mos6502_addr, addr_mode_abx_carry1)
{
    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->X = 3;
    cpu->P = cpu->P | CARRY;
    cr_assert_eq(mos6502_resolve_abx(cpu), 111);
}

Test(mos6502_addr, addr_mode_aby_carry0)
{
    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->Y = 4;
    cr_assert_eq(mos6502_resolve_aby(cpu), 111);
}

Test(mos6502_addr, addr_mode_aby_carry1)
{
    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->Y = 3;
    cpu->P = cpu->P | CARRY;
    cr_assert_eq(mos6502_resolve_aby(cpu), 111);
}

Test(mos6502_addr, addr_mode_imm)
{
    SET_PC_BYTE(cpu, 0, 0x12);
    cr_assert_eq(mos6502_resolve_imm(cpu), 0x12);
}

Test(mos6502_addr, addr_mode_idx)
{
    vm_segment_set(cpu->memory, 0x17, 0x23);
    vm_segment_set(cpu->memory, 0x23, 123);

    SET_PC_BYTE(cpu, 0, 0x12);
    cpu->X = 5;
    cr_assert_eq(mos6502_resolve_idx(cpu), 123);
}

Test(mos6502_addr, addr_mode_idy)
{
    vm_segment_set(cpu->memory, 0x12, 0x23);
    vm_segment_set(cpu->memory, 0x28, 123);

    SET_PC_BYTE(cpu, 0, 0x12);
    cpu->Y = 5;
    cr_assert_eq(mos6502_resolve_idy(cpu), 123);
}

Test(mos6502_addr, addr_mode_ind)
{
    vm_segment_set(cpu->memory, 0x1234, 0x23);
    vm_segment_set(cpu->memory, 0x1235, 0x45);
    vm_segment_set(cpu->memory, 0x2345, 123);

    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x34);
    cr_assert_eq(mos6502_resolve_ind(cpu), 123);
}

Test(mos6502_addr, addr_mode_rel_positive)
{
    cpu->PC = 123;
    SET_PC_BYTE(cpu, 0, 88);
    cr_assert_eq(mos6502_resolve_rel(cpu), 0);
    cr_assert_eq(cpu->last_addr, 211);
}

Test(mos6502_addr, addr_mode_rel_negative)
{
    cpu->PC = 123;
    SET_PC_BYTE(cpu, 0, 216);
    cr_assert_eq(mos6502_resolve_rel(cpu), 0);
    cr_assert_eq(cpu->last_addr, 34);
}

Test(mos6502_addr, addr_mode_zpg)
{
    vm_segment_set(cpu->memory, 0x0034, 222);
    SET_PC_BYTE(cpu, 0, 0x34);
    cr_assert_eq(mos6502_resolve_zpg(cpu), 222);
}

Test(mos6502_addr, addr_mode_zpx)
{
    vm_segment_set(cpu->memory, 0x0034, 222);
    SET_PC_BYTE(cpu, 0, 0x30);
    cpu->X = 4;
    cr_assert_eq(mos6502_resolve_zpx(cpu), 222);
}

Test(mos6502_addr, addr_mode_zpy)
{
    vm_segment_set(cpu->memory, 0x0034, 222);
    SET_PC_BYTE(cpu, 0, 0x2F);
    cpu->Y = 5;
    cr_assert_eq(mos6502_resolve_zpy(cpu), 222);
}

