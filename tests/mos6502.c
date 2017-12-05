#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, create) {
    mos6502 *cpu;

    cpu = mos6502_create();
    cr_assert_neq(cpu, NULL);

    cr_assert_eq(cpu->memory->size, MOS6502_MEMSIZE);

    cr_assert_eq(cpu->PC, 0);
    cr_assert_eq(cpu->A, 0);
    cr_assert_eq(cpu->X, 0);
    cr_assert_eq(cpu->Y, 0);
    cr_assert_eq(cpu->P, 0);
    cr_assert_eq(cpu->S, 0);

    mos6502_free(cpu);
}

Test(mos6502, next_byte) {
    INIT_ADDR_MODE();

    cpu->PC = 128;
    vm_segment_set(cpu->memory, cpu->PC, 123);
    vm_segment_set(cpu->memory, cpu->PC + 1, 234);
    vm_segment_set(cpu->memory, cpu->PC + 2, 12);

    cr_assert_eq(mos6502_next_byte(cpu), 123);
    cr_assert_eq(mos6502_next_byte(cpu), 234);
    cr_assert_eq(mos6502_next_byte(cpu), 12);

    END_ADDR_MODE();
}

Test(mos6502, push_stack)
{
    START_CPU_TEST(mos6502);

    mos6502_push_stack(cpu, 0x1234);
    cr_assert_eq(vm_segment_get(cpu->memory, 0x0100), 0x12);
    cr_assert_eq(vm_segment_get(cpu->memory, 0x0101), 0x34);

    END_CPU_TEST(mos6502);
}

Test(mos6502, pop_stack)
{
    START_CPU_TEST(mos6502);

    mos6502_push_stack(cpu, 0x1234);
    cr_assert_eq(mos6502_pop_stack(cpu), 0x1234);

    END_CPU_TEST(mos6502);
}

Test(mos6502, modify_status)
{
    START_CPU_TEST(mos6502);

    mos6502_modify_status(cpu, NEGATIVE, 130);
    cr_assert_eq(cpu->P & NEGATIVE, NEGATIVE);
    mos6502_modify_status(cpu, NEGATIVE, 123);
    cr_assert_neq(cpu->P & NEGATIVE, NEGATIVE);

    mos6502_modify_status(cpu, OVERFLOW, 123);
    cr_assert_eq(cpu->P & OVERFLOW, OVERFLOW);
    mos6502_modify_status(cpu, OVERFLOW, 44);
    cr_assert_neq(cpu->P & OVERFLOW, OVERFLOW);

    mos6502_modify_status(cpu, CARRY, 23);
    cr_assert_eq(cpu->P & CARRY, CARRY);
    mos6502_modify_status(cpu, CARRY, 0);
    cr_assert_neq(cpu->P & CARRY, CARRY);

    mos6502_modify_status(cpu, ZERO, 0);
    cr_assert_eq(cpu->P & ZERO, ZERO);
    mos6502_modify_status(cpu, ZERO, 1);
    cr_assert_neq(cpu->P & ZERO, ZERO);

    END_CPU_TEST(mos6502);
}

Test(mos6502, set_status)
{
    START_CPU_TEST(mos6502);

    mos6502_set_status(cpu, BREAK | INTERRUPT | DECIMAL);
    cr_assert_eq(cpu->P & (BREAK | INTERRUPT | DECIMAL), BREAK | INTERRUPT | DECIMAL);

    END_CPU_TEST(mos6502);
}

Test(mos6502, instruction)
{
    cr_assert_eq(mos6502_instruction(0x1D), ORA);
    cr_assert_eq(mos6502_instruction(0xD8), CLD);
    cr_assert_eq(mos6502_instruction(0x98), TYA);
}

Test(mos6502, cycles)
{
    START_CPU_TEST(mos6502);

    cr_assert_eq(mos6502_cycles(cpu, 0x76), 6);
    cr_assert_eq(mos6502_cycles(cpu, 0xBA), 2);

    // In this case, we aren't cross a page boundary, and the number of
    // cycles should stay at 4
    cpu->last_addr = 0x5070;
    cpu->X = 23;
    cr_assert_eq(mos6502_cycles(cpu, 0x1D), 4);

    // Testing that crossing a page boundary adds one to the number of
    // cycles
    cpu->X = 200;
    cr_assert_eq(mos6502_cycles(cpu, 0x1D), 5);

    END_CPU_TEST(mos6502);
}
