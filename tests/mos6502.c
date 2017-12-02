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
