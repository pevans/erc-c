#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"

Test(mos6502, lda)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_lda(cpu, 123);
    cr_assert_eq(cpu->A, 123);

    END_CPU_TEST(mos6502);
}

Test(mos6502, ldx)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_ldx(cpu, 123);
    cr_assert_eq(cpu->X, 123);

    END_CPU_TEST(mos6502);
}

Test(mos6502, ldy)
{
    START_CPU_TEST(mos6502);

    mos6502_handle_ldy(cpu, 123);
    cr_assert_eq(cpu->Y, 123);

    END_CPU_TEST(mos6502);
}

Test(mos6502, pha)
{
    START_CPU_TEST(mos6502);

    cpu->A = 0x24;
    mos6502_handle_pha(cpu, 0);

    cr_assert_eq(vm_segment_get(cpu->memory, 0x0100), 0x00);
    cr_assert_eq(vm_segment_get(cpu->memory, 0x0101), 0x24);

    END_CPU_TEST(mos6502);
}

Test(mos6502, php)
{
    START_CPU_TEST(mos6502);

    cpu->P = 0x43;
    mos6502_handle_php(cpu, 0);

    cr_assert_eq(vm_segment_get(cpu->memory, 0x0100), 0x00);
    cr_assert_eq(vm_segment_get(cpu->memory, 0x0101), 0x43);

    END_CPU_TEST(mos6502);
}

Test(mos6502, pla)
{
    START_CPU_TEST(mos6502);

    mos6502_push_stack(cpu, 0x0033);
    mos6502_handle_pla(cpu, 0);

    cr_assert_eq(cpu->A, 0x33);

    END_CPU_TEST(mos6502);
}

Test(mos6502, plp)
{
    START_CPU_TEST(mos6502);

    mos6502_push_stack(cpu, 0x0052);
    mos6502_handle_plp(cpu, 0);

    cr_assert_eq(cpu->P, 0x52);

    END_CPU_TEST(mos6502);
}

Test(mos6502, sta)
{
    START_CPU_TEST(mos6502);

    cpu->A = 123;
    cpu->last_addr = 555;
    mos6502_handle_sta(cpu, 0);
    cr_assert_eq(vm_segment_get(cpu->memory, cpu->last_addr), cpu->A);

    END_CPU_TEST(mos6502);
}

Test(mos6502, stx)
{
    START_CPU_TEST(mos6502);

    cpu->X = 222;
    cpu->last_addr = 444;
    mos6502_handle_stx(cpu, 0);
    cr_assert_eq(vm_segment_get(cpu->memory, cpu->last_addr), cpu->X);

    END_CPU_TEST(mos6502);
}

Test(mos6502, sty)
{
    START_CPU_TEST(mos6502);

    cpu->Y = 111;
    cpu->last_addr = 253;
    mos6502_handle_sty(cpu, 0);
    cr_assert_eq(vm_segment_get(cpu->memory, cpu->last_addr), cpu->Y);

    END_CPU_TEST(mos6502);
}

Test(mos6502, tax)
{
    START_CPU_TEST(mos6502);

    cpu->A = 111;
    cpu->X = 222;
    mos6502_handle_tax(cpu, 0);
    cr_assert_eq(cpu->X, 111);

    END_CPU_TEST(mos6502);
}

Test(mos6502, tay)
{
    START_CPU_TEST(mos6502);

    cpu->A = 111;
    cpu->Y = 115;
    mos6502_handle_tay(cpu, 0);
    cr_assert_eq(cpu->Y, 111);

    END_CPU_TEST(mos6502);
}

Test(mos6502, tsx)
{
    START_CPU_TEST(mos6502);

    cpu->S = 111;
    cpu->X = 222;
    mos6502_handle_tsx(cpu, 0);
    cr_assert_eq(cpu->X, 111);

    END_CPU_TEST(mos6502);
}

Test(mos6502, txa)
{
    START_CPU_TEST(mos6502);

    cpu->A = 111;
    cpu->X = 222;
    mos6502_handle_txa(cpu, 0);
    cr_assert_eq(cpu->A, 222);

    END_CPU_TEST(mos6502);
}

Test(mos6502, txs)
{
    START_CPU_TEST(mos6502);

    cpu->S = 111;
    cpu->X = 222;
    mos6502_handle_txs(cpu, 0);
    cr_assert_eq(cpu->S, 222);

    END_CPU_TEST(mos6502);
}

Test(mos6502, tya)
{
    START_CPU_TEST(mos6502);

    cpu->A = 111;
    cpu->Y = 222;
    mos6502_handle_tya(cpu, 0);
    cr_assert_eq(cpu->A, 222);

    END_CPU_TEST(mos6502);
}
