#ifndef _MOS6502_TESTS_H
#define _MOS6502_TESTS_H

static mos6502 *cpu;

static void
setup()
{
    cpu = mos6502_create();
}

static void
teardown()
{
    mos6502_free(cpu);
}

#endif
