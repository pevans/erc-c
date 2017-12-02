#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, get_address_resolver) {
    INIT_ADDR_MODE();

    cr_assert_eq(mos6502_get_address_resolver(ACC), mos6502_resolve_acc);
    cr_assert_eq(mos6502_get_address_resolver(ABS), mos6502_resolve_abs);
    cr_assert_eq(mos6502_get_address_resolver(ABX), mos6502_resolve_abx);
    cr_assert_eq(mos6502_get_address_resolver(ABY), mos6502_resolve_aby);
    cr_assert_eq(mos6502_get_address_resolver(IMM), mos6502_resolve_imm);
    cr_assert_eq(mos6502_get_address_resolver(IND), mos6502_resolve_ind);
    cr_assert_eq(mos6502_get_address_resolver(IDX), mos6502_resolve_idx);
    cr_assert_eq(mos6502_get_address_resolver(IDY), mos6502_resolve_idy);
    cr_assert_eq(mos6502_get_address_resolver(REL), mos6502_resolve_rel);
    cr_assert_eq(mos6502_get_address_resolver(ZPG), mos6502_resolve_zpg);
    cr_assert_eq(mos6502_get_address_resolver(ZPX), mos6502_resolve_zpx);
    cr_assert_eq(mos6502_get_address_resolver(ZPY), mos6502_resolve_zpy);

    // Trick question: implied mode doesn't require an operand, so there
    // should be no possible resolution with it.
    cr_assert_eq(mos6502_get_address_resolver(IMP), NULL);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_acc) {
    INIT_ADDR_MODE();

    cpu->A = 123;
    cr_assert_eq(mos6502_resolve_acc(cpu), 123);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_abs) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x34);
    cr_assert_eq(mos6502_resolve_abs(cpu), 111);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_abx_carry0) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->X = 4;
    cr_assert_eq(mos6502_resolve_abx(cpu), 111);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_abx_carry1) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->X = 3;
    cpu->P = cpu->P | CARRY;
    cr_assert_eq(mos6502_resolve_abx(cpu), 111);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_aby_carry0) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->Y = 4;
    cr_assert_eq(mos6502_resolve_aby(cpu), 111);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_aby_carry1) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x1234, 111);
    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x30);
    cpu->Y = 3;
    cpu->P = cpu->P | CARRY;
    cr_assert_eq(mos6502_resolve_aby(cpu), 111);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_imm) {
    INIT_ADDR_MODE();

    SET_PC_BYTE(cpu, 0, 0x12);
    cr_assert_eq(mos6502_resolve_imm(cpu), 0x12);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_idx) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x17, 0x23);
    vm_segment_set(cpu->memory, 0x23, 123);

    SET_PC_BYTE(cpu, 0, 0x12);
    cpu->X = 5;
    cr_assert_eq(mos6502_resolve_idx(cpu), 123);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_idy) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x12, 0x23);
    vm_segment_set(cpu->memory, 0x28, 123);

    SET_PC_BYTE(cpu, 0, 0x12);
    cpu->Y = 5;
    cr_assert_eq(mos6502_resolve_idy(cpu), 123);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_ind) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x1234, 0x23);
    vm_segment_set(cpu->memory, 0x1235, 0x45);
    vm_segment_set(cpu->memory, 0x2345, 123);

    SET_PC_BYTE(cpu, 0, 0x12);
    SET_PC_BYTE(cpu, 1, 0x34);
    cr_assert_eq(mos6502_resolve_ind(cpu), 123);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_rel_positive) {
    INIT_ADDR_MODE();

    cpu->PC = 123;
    SET_PC_BYTE(cpu, 0, 88);
    cr_assert_eq(mos6502_resolve_rel(cpu), 0);
    cr_assert_eq(cpu->last_addr, 211);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_rel_negative) {
    INIT_ADDR_MODE();

    cpu->PC = 123;
    SET_PC_BYTE(cpu, 0, 216);
    cr_assert_eq(mos6502_resolve_rel(cpu), 0);
    cr_assert_eq(cpu->last_addr, 34);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_zpg) {
    INIT_ADDR_MODE();

    vm_segment_set(cpu->memory, 0x0034, 222);
    SET_PC_BYTE(cpu, 0, 0x34);
    cr_assert_eq(mos6502_resolve_zpg(cpu), 222);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_zpx) {
    INIT_ADDR_MODE();
    
    vm_segment_set(cpu->memory, 0x0034, 222);
    SET_PC_BYTE(cpu, 0, 0x30);
    cpu->X = 4;
    cr_assert_eq(mos6502_resolve_zpx(cpu), 222);

    END_ADDR_MODE();
}

Test(mos6502, addr_mode_zpy) {
    INIT_ADDR_MODE();
    
    vm_segment_set(cpu->memory, 0x0034, 222);
    SET_PC_BYTE(cpu, 0, 0x2F);
    cpu->Y = 5;
    cr_assert_eq(mos6502_resolve_zpy(cpu), 222);

    END_ADDR_MODE();
}

