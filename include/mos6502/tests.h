#ifndef _MOS6502_TESTS_H
#define _MOS6502_TESTS_H

static mos6502 *cpu;
static vm_segment *mem;

static void
setup()
{
    mem = vm_segment_create(MOS6502_MEMSIZE);
    cpu = mos6502_create(mem, mem);
}

static void
teardown()
{
    mos6502_free(cpu);
    vm_segment_free(mem);
}

#endif
