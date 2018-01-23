#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502, .init = setup, .fini = teardown);

/* Test(mos6502, free) */

Test(mos6502, create)
{
    cr_assert_neq(cpu, NULL);

    cr_assert_eq(cpu->rmem->size, MOS6502_MEMSIZE);
    cr_assert_eq(cpu->wmem->size, MOS6502_MEMSIZE);

    cr_assert_eq(cpu->PC, 0);
    cr_assert_eq(cpu->A, 0);
    cr_assert_eq(cpu->X, 0);
    cr_assert_eq(cpu->Y, 0);
    cr_assert_eq(cpu->P, MOS_STATUS_DEFAULT);
    cr_assert_eq(cpu->S, 0xff);
}

Test(mos6502, push_stack)
{
    mos6502_push_stack(cpu, 0x34);
    cr_assert_eq(mos6502_get(cpu, 0x0100), 0x34);
}

Test(mos6502, pop_stack)
{
    mos6502_push_stack(cpu, 0x34);
    cr_assert_eq(mos6502_pop_stack(cpu), 0x34);
}

Test(mos6502, modify_status)
{
    mos6502_modify_status(cpu, MOS_NEGATIVE, 130, 130);
    cr_assert_eq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);
    mos6502_modify_status(cpu, MOS_NEGATIVE, 123, 123);
    cr_assert_neq(cpu->P & MOS_NEGATIVE, MOS_NEGATIVE);

    mos6502_modify_status(cpu, MOS_OVERFLOW, 123, 133);
    cr_assert_eq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);
    mos6502_modify_status(cpu, MOS_OVERFLOW, 44, 44);
    cr_assert_neq(cpu->P & MOS_OVERFLOW, MOS_OVERFLOW);

    mos6502_modify_status(cpu, MOS_CARRY, 230, 230);
    cr_assert_eq(cpu->P & MOS_CARRY, MOS_CARRY);
    mos6502_modify_status(cpu, MOS_CARRY, 30, 190);
    cr_assert_neq(cpu->P & MOS_CARRY, MOS_CARRY);

    mos6502_modify_status(cpu, MOS_ZERO, 0, 0);
    cr_assert_eq(cpu->P & MOS_ZERO, MOS_ZERO);
    mos6502_modify_status(cpu, MOS_ZERO, 1, 1);
    cr_assert_neq(cpu->P & MOS_ZERO, MOS_ZERO);
}

Test(mos6502, set_status)
{
    mos6502_set_status(cpu, MOS_BREAK | MOS_INTERRUPT | MOS_DECIMAL);
    cr_assert_eq(cpu->P & (MOS_BREAK | MOS_INTERRUPT | MOS_DECIMAL), MOS_BREAK | MOS_INTERRUPT | MOS_DECIMAL);
}

Test(mos6502, instruction)
{
    cr_assert_eq(mos6502_instruction(0x1D), ORA);
    cr_assert_eq(mos6502_instruction(0xD8), CLD);
    cr_assert_eq(mos6502_instruction(0x98), TYA);
}

Test(mos6502, cycles)
{
    cr_assert_eq(mos6502_cycles(cpu, 0x76), 6);
    cr_assert_eq(mos6502_cycles(cpu, 0xBA), 2);

    // In this case, we aren't cross a page boundary, and the number of
    // cycles should stay at 4
    cpu->eff_addr = 0x5070;
    cpu->X = 23;
    cr_assert_eq(mos6502_cycles(cpu, 0x1D), 4);

    // Testing that crossing a page boundary adds one to the number of
    // cycles
    cpu->X = 200;
    cr_assert_eq(mos6502_cycles(cpu, 0x1D), 5);
}

Test(mos6502, get_instruction_handler)
{
    cr_assert_eq(mos6502_get_instruction_handler(0x00), mos6502_handle_brk);
    cr_assert_eq(mos6502_get_instruction_handler(0x1D), mos6502_handle_ora);
    cr_assert_eq(mos6502_get_instruction_handler(0x20), mos6502_handle_jsr);
}

Test(mos6502, execute)
{
    mos6502_set(cpu, 11, 34);
    mos6502_set(cpu, 10, 0x69);
    cpu->PC = 10;

    // Make sure we don't have carry turned on, or else we'll get 35!
    cpu->P &= ~MOS_CARRY;

    mos6502_execute(cpu);
    cr_assert_eq(cpu->A, 34);
}

Test(mos6502, would_jump)
{
    bool expect;
    for (int inst = 0; inst <= TYA; inst++) {
        switch (inst) {
            case BCC:
            case BCS:
            case BEQ:
            case BMI:
            case BNE:
            case BPL:
            case BRK:
            case BVC:
            case BVS:
            case JMP:
            case JSR:
            case RTS:
            case RTI:
                expect = true;
                break;

            default:
                expect = false;
                break;
        }

        cr_assert_eq(mos6502_would_jump(inst), expect);
    }
}

Test(mos6502, get_address_resolver)
{
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
}


Test(mos6502, get)
{
    vm_segment_set(cpu->wmem, 0, 123);
    cr_assert_eq(mos6502_get(cpu, 0), 123);
}

Test(mos6502, get16)
{
    vm_segment_set16(cpu->wmem, 0, 0x3344);
    cr_assert_eq(mos6502_get16(cpu, 0), 0x3344);
}

Test(mos6502, set)
{
    mos6502_set(cpu, 0, 111);
    cr_assert_eq(vm_segment_get(cpu->rmem, 0), 111);
}

Test(mos6502, set16)
{
    mos6502_set16(cpu, 0, 0x2255);
    cr_assert_eq(vm_segment_get16(cpu->rmem, 0), 0x2255);
}

Test(mos6502, set_memory)
{
    vm_segment *rmem, *wmem;

    rmem = (vm_segment *)111;
    wmem = (vm_segment *)222;

    mos6502_set_memory(cpu, rmem, wmem);

    cr_assert_eq(cpu->rmem, rmem);
    cr_assert_eq(cpu->wmem, wmem);
}

Test(mos6502, last_executed)
{
    vm_8bit opcode, operand;
    vm_16bit addr;

    mos6502_set(cpu, 0, 0xA9);      // LDA #$EE
    mos6502_set(cpu, 1, 0xEE);
    mos6502_set(cpu, 2, 0x8D);      // STA $1234
    mos6502_set16(cpu, 3, 0x1234);

    mos6502_execute(cpu);
    mos6502_last_executed(cpu, &opcode, &operand, NULL);
    cr_assert_eq(opcode, 0xA9);
    cr_assert_eq(operand, 0xEE);

    mos6502_execute(cpu);
    mos6502_last_executed(cpu, &opcode, NULL, &addr);
    cr_assert_eq(opcode, 0x8D);
    cr_assert_eq(addr, 0x1234);
}
