#include <criterion/criterion.h>

#include "mos6502/mos6502.h"
#include "mos6502/enums.h"
#include "mos6502/tests.h"

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

    cr_assert_eq(mos6502_get(cpu, 0x01ff), 0x24);
}

Test(mos6502_loadstor, php)
{
    cpu->P = 0x43;
    mos6502_handle_php(cpu, 0);

    cr_assert_eq(mos6502_get(cpu, 0x01ff), 0x43);
}

Test(mos6502_loadstor, phx)
{
    cpu->X = 123;
    mos6502_handle_phx(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, 0x01ff), 123);
}

Test(mos6502_loadstor, phy)
{
    cpu->Y = 234;
    mos6502_handle_phy(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, 0x01ff), 234);
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

Test(mos6502_loadstor, plx)
{
    mos6502_push_stack(cpu, 87);
    mos6502_handle_plx(cpu, 0);

    cr_assert_eq(cpu->X, 87);
}

Test(mos6502_loadstor, ply)
{
    mos6502_push_stack(cpu, 44);
    mos6502_handle_ply(cpu, 0);

    cr_assert_eq(cpu->Y, 44);
}

Test(mos6502_loadstor, sta)
{
    cpu->A = 123;
    cpu->eff_addr = 555;
    mos6502_handle_sta(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, cpu->eff_addr), cpu->A);
}

Test(mos6502_loadstor, stx)
{
    cpu->X = 222;
    cpu->eff_addr = 444;
    mos6502_handle_stx(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, cpu->eff_addr), cpu->X);
}

Test(mos6502_loadstor, sty)
{
    cpu->Y = 111;
    cpu->eff_addr = 253;
    mos6502_handle_sty(cpu, 0);
    cr_assert_eq(mos6502_get(cpu, cpu->eff_addr), cpu->Y);
}

Test(mos6502_loadstor, stz)
{
    // To begin with, we want a non-zero value in eff_addr
    cpu->eff_addr = 111;
    mos6502_set(cpu, cpu->eff_addr, 222);

    // Furthermore, we pass in a non-zero operand to stz, which
    // _shouldn't_ care what the operand is. It should only assign a
    // zero to eff_addr.
    mos6502_handle_stz(cpu, 11);

    cr_assert_eq(mos6502_get(cpu, cpu->eff_addr), 0);
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
