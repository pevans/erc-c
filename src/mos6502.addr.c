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
    REL, IDY, ZPG, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, ACC, NOA, NOA, ABX, ABX, NOA, // 1x
    ABS, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, ABS, ABS, ABS, NOA, // 2x
    REL, IDY, ZPG, NOA, ZPX, ZPX, ZPX, NOA, IMP, ABY, ACC, NOA, ABX, ABX, ABX, NOA, // 3x
    IMP, IDX, NOA, NOA, NOA, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, ABS, ABS, ABS, NOA, // 4x
    REL, IDY, ZPG, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // 5x
    IMP, IDX, NOA, NOA, NOA, ZPG, ZPG, NOA, IMP, IMM, ACC, NOA, IND, ABS, ABS, NOA, // 6x
    REL, IDY, ZPG, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, ABX, ABX, ABX, NOA, // 7x
    NOA, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // 8x
    REL, IDY, ZPG, NOA, ZPX, ZPX, ZPY, NOA, IMP, ABY, IMP, NOA, NOA, ABX, NOA, NOA, // 9x
    IMM, IDX, IMM, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // Ax
    REL, IDY, ZPG, NOA, ZPX, ZPX, ZPY, NOA, IMP, ABY, IMP, NOA, ABX, ABX, ABY, NOA, // Bx
    IMM, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // Cx
    REL, IDY, ZPG, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // Dx
    IMM, IDX, NOA, NOA, ZPG, ZPG, ZPG, NOA, IMP, IMM, IMP, NOA, ABS, ABS, ABS, NOA, // Ex
    REL, IDY, ZPG, NOA, NOA, ZPX, ZPX, NOA, IMP, ABY, NOA, NOA, NOA, ABX, ABX, NOA, // Fx
};

/*
 * Just a little macro to help us figure out what the address is for
 * for 16-bit values
 */
#define ADDR_HILO(cpu) \
    vm_16bit addr; \
    vm_8bit hi, lo; \
    lo = mos6502_get(cpu, cpu->PC + 1); \
    hi = mos6502_get(cpu, cpu->PC + 2); \
    addr = (hi << 8) | lo

/*
 * In contrast to the ADDR_HILO macro, here we want just one byte from
 * the current program counter, and it is the (only) significant byte.
 */
#define ADDR_LO(cpu) \
    vm_16bit addr; \
    addr = mos6502_get(cpu, cpu->PC + 1)

/*
 * This will both define the `eff_addr` variable (which is the effective
 * address) and assign that value to the `eff_addr` field of the cpu.
 */
#define EFF_ADDR(addr) \
    vm_16bit eff_addr = addr; \
    cpu->eff_addr = eff_addr

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
    return mos6502_get(cpu, addr);
}

/*
 * The absolute x-indexed address mode is a slight modification of the
 * absolute mode. Here, we consume two bytes, but add the X register
 * value to what we read. This is a mode you would use if you were
 * scanning a table, for instance.
 */
DEFINE_ADDR(abx)
{
    ADDR_HILO(cpu);
    EFF_ADDR(addr + cpu->X);

    return mos6502_get(cpu, eff_addr);
}

/*
 * Very much the mirror opposite of the ABX address mode; the only
 * difference is we use the Y register, not the X.
 */
DEFINE_ADDR(aby)
{
    ADDR_HILO(cpu);
    EFF_ADDR(addr + cpu->Y);

    return mos6502_get(cpu, eff_addr);
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
    return mos6502_get(cpu, cpu->PC + 1);
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

    ind_lo = mos6502_get(cpu, addr);
    ind_hi = mos6502_get(cpu, addr + 1);
    EFF_ADDR((ind_hi << 8) | ind_lo);

    return mos6502_get(cpu, eff_addr);
}

/*
 * The indirect x-indexed address mode, as well as the y-indexed mode,
 * are a bit complicated. The single, next byte we read is a zero-page
 * address to the base of _another_ zero-page address in memory; we add
 * X to it, which is the address of what we next dereference.
 */
DEFINE_ADDR(idx)
{
    vm_8bit addr;
    vm_16bit caddr;

    // The address we are given as an operand must be immediately
    // incremented by the content of the X register.
    addr = mos6502_get(cpu, cpu->PC + 1) + cpu->X;

    // And the combined address will then be the point of the LSB to a
    // 16-bit pointer; so addr+1 holds the MSB, and we combined it in
    // the usual, little-endian way.
    caddr = (mos6502_get(cpu, addr + 1) << 8) | mos6502_get(cpu, addr);

    // And that's really it--that's our effective address.
    EFF_ADDR(caddr);

    return mos6502_get(cpu, eff_addr);
}

/*
 * In significant contrast, the y-indexed indirect mode will read a
 * zero-page address from the next byte, and dereference it immediately.
 * The ensuing address will then have Y added to it, and then
 * dereferenced for the final time.
 */
DEFINE_ADDR(idy)
{
    vm_8bit addr;
    vm_16bit caddr;     // combined address

    // The immediate address we know is the operand following the
    // opcode.
    addr = mos6502_get(cpu, cpu->PC + 1);

    // But that's just the first part of the combined pointer address we
    // care about; caddr therefore is the combined address pointed at by
    // addr and addr + 1, with respect to little-endian order (ergo
    // mem[addr+1] is the MSB, mem[addr] is the LSB).
    caddr = (mos6502_get(cpu, addr + 1) << 8) | mos6502_get(cpu, addr);

    // But that's not all! We also need to increment that combined
    // address by the content of the Y register.
    EFF_ADDR(caddr + cpu->Y);

    return mos6502_get(cpu, eff_addr);
}

/*
 * The relative mode means we want to return an address in
 * memory which is relative to PC. If bit 7 is 1, which
 * means if addr > 127, then we treat the operand as though it
 * were negative. 
 */
DEFINE_ADDR(rel)
{
    vm_16bit reladdr;

    ADDR_LO(cpu);
    reladdr = cpu->PC + addr + 2;

    if (addr > 127) {
        // If the address has the 8th bit high, then we treat the
        // relative number as signed; we then subtract 256 from whatever
        // the addition was with the operand (addr, in this case).
        reladdr -= 256;
    }

    // But if not, then we can let the addition done above stand.
    EFF_ADDR(reladdr);
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

    return mos6502_get(cpu, eff_addr);
}

/*
 * In zero-page x-indexed mode, we read the next byte; add X to that;
 * and dereference the result.
 */
DEFINE_ADDR(zpx)
{
    ADDR_LO(cpu);
    EFF_ADDR((addr + cpu->X) & 0xff);

    return mos6502_get(cpu, eff_addr);
}

/*
 * This is, as with absolute y-indexed mode, the mirror opposite of the
 * zero-page x-indexed mode. We simply use the Y register and not the X.
 */
DEFINE_ADDR(zpy)
{
    ADDR_LO(cpu);
    EFF_ADDR((addr + cpu->Y) & 0xff);

    return mos6502_get(cpu, eff_addr);
}
