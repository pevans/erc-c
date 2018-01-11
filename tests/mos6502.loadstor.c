#include <criterion/criterion.h>

#include "mos6502.h"
#include "mos6502.enums.h"
#include "mos6502.tests.h"

TestSuite(mos6502_loadstor, .init = setup, .fini = teardown);

Test(mos6502_loadstor, lda)
{
    mos6502_handle_lda(cpu, 123);
    cr_assert_eq(cpu->A, 123);
}

Test(mos6502_loadstor, ldx)
{
    mos6502_handle_ldx(cpu, 123);
    cr_assert_eq(cpu->X, 123);
}

Test(mos6502_loadstor, ldy)
{
    mos6502_handle_ldy(cpu, 123);
    cr_assert_eq(cpu->Y, 123);
}

Test(mos6502_loadstor, pha)
{
    cpu->A = 0x24;
    mos6502_handle_pha(cpu, 0);

    cr_assert_eq(mos6502_get(cpu, 0x0100), 0x24);
    cr_assert_eq(mos6502_get(cpu, 0x0101), 0x00);
}

Test(mos6502_loadstor, php)
{
    cpu->P = 0x43;
    mos6502_handle_php(cpu, 0);

    cr_assert_eq(mos6502_get(cpu, 0x0100), 0x43);
    cr_assert_eq(mos6502_get(cpu, 0x0101), 0x00);
}

Test(mos6502_loadstor, pla)
{
    mos6502_push_stack(cpu, 0x0033);
    mos6502_handle_pla(cpu, 0);

    cr_assert_eq(cpu->A, 0x33);
}

Test(mos6502_loadstor, plp)
{
    mos6502_push_stack(cpu, 0x0052);
    mos6502_handle_plp(cpu, 0);

    cr_assert_eq(cpu->P, 0x52);
}

Test(mos6502_loadstor, sta)
{
    cpu->A = 123;
    cpu->last_addr = 555;
    mos6502_handle_sta(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, cpu->last_addr), cpu->A);
}

Test(mos6502_loadstor, stx)
{
    cpu->X = 222;
    cpu->last_addr = 444;
    mos6502_handle_stx(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, cpu->last_addr), cpu->X);
}

Test(mos6502_loadstor, sty)
{
    cpu->Y = 111;
    cpu->last_addr = 253;
    mos6502_handle_sty(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, cpu->last_addr), cpu->Y);
}

Test(mos6502_loadstor, tax)
{
    cpu->A = 111;
    cpu->X = 222;
    mos6502_handle_tax(cpu, 0);
    cr_assert_eq(cpu->X, 111);
}

Test(mos6502_loadstor, tay)
{
    cpu->A = 111;
    cpu->Y = 115;
    mos6502_handle_tay(cpu, 0);
    cr_assert_eq(cpu->Y, 111);
}

Test(mos6502_loadstor, tsx)
{
    cpu->S = 111;
    cpu->X = 222;
    mos6502_handle_tsx(cpu, 0);
    cr_assert_eq(cpu->X, 111);
}

Test(mos6502_loadstor, txa)
{
    cpu->A = 111;
    cpu->X = 222;
    mos6502_handle_txa(cpu, 0);
    cr_assert_eq(cpu->A, 222);
}

Test(mos6502_loadstor, txs)
{
    cpu->S = 111;
    cpu->X = 222;
    mos6502_handle_txs(cpu, 0);
    cr_assert_eq(cpu->S, 222);
}

Test(mos6502_loadstor, tya)
{
    cpu->A = 111;
    cpu->Y = 222;
    mos6502_handle_tya(cpu, 0);
    cr_assert_eq(cpu->A, 222);
}
