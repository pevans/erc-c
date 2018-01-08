/*
 * mos6502.addr.c
 *
 * Here we have support for the address modes that are built into the
 * MOS 6502 chip. In general, these address modes help the instruction
 * figure out _what_ it is working with, which is either a value from a
 * register, or from some place in memory.
 */

#include <stdlib.h>

#include "mos6502.h"
#include "mos6502.enums.h"

/*
 * This is a table of all the possible opcodes the 6502 understands,
 * mapped to the correct address mode. (Well -- I _hope_ it's the
 * correct address mode!)
 */
static int addr_modes[] = {
//   00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F
    IMP, IDX, NOA, NOA, NOA, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, NOA, ABS, ABS, NOA, // 0x
    REL, IDY, NOA, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // 1x
    ABS, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, ABS, ABS, ABS, NOA, // 2x
    REL, IDY, NOA, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // 3x
    IMP, IDX, NOA, NOA, NOA, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, ABS, ABS, ABS, NOA, // 4x
    REL, IDY, NOA, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // 5x
    IMP, IDX, NOA, NOA, NOA, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, IND, ABS, ABS, NOA, // 6x
    REL, IDY, NOA, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // 7x
    NOA, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, NOA, IMP, NOA, ABS, ABS, ABS, NOA, // 8x
    REL, IDY, NOA, NOA, ZPX, ZPX, ZPY, NOA, IMP, ABY, IMP, NOA, NOA, ABX, NOA, NOA, // 9x
    IMM, IDX, IMM, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // Ax
    REL, IDY, NOA, NOA, ZPX, ZPX, ZPY, NOA, IMP, ABY, IMP, NOA, ABX, ABX, ABY, NOA, // Bx
    IMM, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // Cx
    REL, IDY, NOA, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // Dx
    IMM, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // Ex
    REL, IDY, NOA, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // Fx
};

/*
 * Just a little macro to help us figure out what the address is for
 * for 16-bit values
 */
#define ADDR_HILO(cpu) \
    vm_16bit addr; \
    vm_8bit hi, lo; \
    lo = mos6502_next_byte(cpu); \
    hi = mos6502_next_byte(cpu); \
    addr = (hi << 8) | lo

/*
 * In contrast to the ADDR_HILO macro, here we want just one byte from
 * the current program counter, and it is the (only) significant byte.
 */
#define ADDR_LO(cpu) \
    vm_16bit addr; \
    addr = mos6502_next_byte(cpu)

/*
 * This will both define the `eff_addr` variable (which is the effective
 * address) and assign that value to the `last_addr` field of the cpu.
 */
#define EFF_ADDR(addr) \
    vm_16bit eff_addr = addr; \
    cpu->last_addr = eff_addr

/*
 * A tiny convenience macro to help us define address resolver
 * functions.
 */
#define DEFINE_ADDR(mode) \
    vm_8bit mos6502_resolve_##mode (mos6502 *cpu)

/*
 * Return the address mode for a given opcode.
 */
int
mos6502_addr_mode(vm_8bit opcode)
{
    return addr_modes[opcode];
}

/*
 * In the ACC address mode, the instruction will consider just the A
 * register. (It's probably the simplest resolution mode for us to
 * execute.)
 */
DEFINE_ADDR(acc)
{
    EFF_ADDR(0);
    return cpu->A;
}

/*
 * This is the absolute address mode. The next two bytes are the address
 * in memory at which our looked-for value resides, so we consume those
 * bytes and return the value located therein.
 */
DEFINE_ADDR(abs)
{
    ADDR_HILO(cpu);
    EFF_ADDR(addr);
    return vm_segment_get(cpu->memory, addr);
}

/*
 * The absolute x-indexed address mode is a slight modification of the
 * absolute mode. Here, we consume two bytes, but add the X register
 * value to what we read -- plus one if we have the carry bit set. This
 * is a mode you would use if you were scanning a table, for instance.
 */
DEFINE_ADDR(abx)
{
    ADDR_HILO(cpu);
    MOS_CARRY_BIT();
    EFF_ADDR(addr + cpu->X + carry);

    return vm_segment_get(cpu->memory, eff_addr);
}

/*
 * Very much the mirror opposite of the ABX address mode; the only
 * difference is we use the Y register, not the X.
 */
DEFINE_ADDR(aby)
{
    ADDR_HILO(cpu);
    MOS_CARRY_BIT();
    EFF_ADDR(addr + cpu->Y + carry);

    return vm_segment_get(cpu->memory, eff_addr);
}

/*
 * In immediate mode, the very next byte is the literal value to be used
 * in the instruction. This is a mode you would use if, for instance,
 * you wanted to say "foo + 5"; 5 would be the operand we return from
 * here.
 */
DEFINE_ADDR(imm)
{
    EFF_ADDR(0);
    return mos6502_next_byte(cpu);
}

/*
 * In indirect mode, we presume that the next two bytes are an address
 * at which _another_ pointer can be found. So we dereference these next
 * two bytes, then dereference the two bytes found at that point, and
 * _that_ is what our value will be.
 */
DEFINE_ADDR(ind)
{
    vm_8bit ind_hi, ind_lo;

    ADDR_HILO(cpu);

    ind_lo = vm_segment_get(cpu->memory, addr);
    ind_hi = vm_segment_get(cpu->memory, addr + 1);
    EFF_ADDR((ind_hi << 8) | ind_lo);

    return vm_segment_get(cpu->memory, eff_addr);
}

/*
 * The indirect x-indexed address mode, as well as the y-indexed mode,
 * are a bit complicated. The single, next byte we read is a zero-page
 * address to the base of _another_ zero-page address in memory; we add
 * X to it, which is the address of what we next dereference. Carry does
 * not factor into the arithmetic.
 */
DEFINE_ADDR(idx)
{
    ADDR_LO(cpu);
    EFF_ADDR(addr + cpu->X);

    return vm_segment_get(
        cpu->memory, 
        vm_segment_get(cpu->memory, eff_addr));
}

/*
 * In significant contrast, the y-indexed indirect mode will read a
 * zero-page address from the next byte, and dereference it immediately.
 * The ensuing address will then have Y added to it, and then
 * dereferenced for the final time. Carry _is_ factored in here.
 */
DEFINE_ADDR(idy)
{
    ADDR_LO(cpu);
    MOS_CARRY_BIT();
    EFF_ADDR(vm_segment_get(cpu->memory, addr) + cpu->Y + carry);

    return vm_segment_get(cpu->memory, eff_addr);
}

/*
 * The relative mode means we want to return an address in
 * memory which is relative to PC. If bit 7 is 1, which
 * means if addr > 127, then we treat the operand as though it
 * were negative. 
 */
DEFINE_ADDR(rel)
{
    vm_16bit orig_pc;

    orig_pc = cpu->PC;
    ADDR_LO(cpu);

    if (addr > 127) {
        // e.g. if lo == 128, then cpu->PC + 127 - lo is the
        // same as subtracting 1 from PC.
        EFF_ADDR(orig_pc + 127 - addr);
        return 0;
    }

    // Otherwise lo is a positive offset from PC
    EFF_ADDR(orig_pc + addr);
    return 0;
}

/*
 * Zero page mode is very straightforward. It's very much the same as
 * absolute mode, except we consider just the next byte, and dereference
 * that (which is, by convention, always going to be an address in the
 * zero page of memory).
 */
DEFINE_ADDR(zpg)
{
    ADDR_LO(cpu);
    EFF_ADDR(addr);

    return vm_segment_get(cpu->memory, eff_addr);
}

/*
 * In zero-page x-indexed mode, we read the next byte; add X to that;
 * and dereference the result. Carry is not a factor here.
 */
DEFINE_ADDR(zpx)
{
    ADDR_LO(cpu);
    EFF_ADDR(addr + cpu->X);

    return vm_segment_get(cpu->memory, eff_addr);
}

/*
 * This is, as with absolute y-indexed mode, the mirror opposite of the
 * zero-page x-indexed mode. We simply use the Y register and not the X,
 * and here as well, we do not factor in the carry bit.
 */
DEFINE_ADDR(zpy)
{
    ADDR_LO(cpu);
    EFF_ADDR(addr + cpu->Y);

    return vm_segment_get(cpu->memory, eff_addr);
}
