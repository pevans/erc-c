/*
 * Ideas:
 *
 * The mos6502 code would _just_ emulate said chip. It would not be a
 * technical part of the computer, and it would in other words be
 * decoupled from the notion of a commadore, apple ii, etc.
 *
 * What you need to do in order to emulate the chip is be able to know
 * about _memory_, and know about _registers_. Things like disk drives,
 * screens, etc. are sort of beyond its knowledge. But memory and
 * registers must be _local_ to the chip's workings; it must be able to
 * directly modify those, as well as share memory/registers/etc. with
 * other parts of a platform.
 *
 * Observations:
 *   - there can only be one chip at a given time; therefore we can get
 *   away with some kind of singleton to represent the chip
 *   - registers and memory need to be available to the chip, but the
 *   chip should not know about the larger platform; we should have
 *   pointers to all of that in the chip structure
 */

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "mos6502.h"

// All of our address modes, instructions, etc. are defined here.
#include "mos6502.enums.h"

static int instructions[] = {
//   00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F
    BRK, ORA, NOP, NOP, NOP, ORA, ASL, NOP, PHP, ORA, ASL, NOP, NOP, ORA, ASL, NOP, // 0x
    BPL, ORA, NOP, NOP, NOP, ORA, ASL, NOP, CLC, ORA, NOP, NOP, NOP, ORA, ASL, NOP, // 1x
    JSR, AND, NOP, NOP, BIT, AND, ROL, NOP, PLP, AND, ROL, NOP, BIT, AND, ROL, NOP, // 2x
    BMI, AND, NOP, NOP, NOP, AND, ROL, NOP, SEC, AND, NOP, NOP, NOP, AND, ROL, NOP, // 3x
    RTI, EOR, NOP, NOP, NOP, EOR, LSR, NOP, PHA, ADC, LSR, NOP, JMP, EOR, LSR, NOP, // 4x
    BVC, EOR, NOP, NOP, NOP, EOR, LSR, NOP, CLI, EOR, NOP, NOP, NOP, EOR, LSR, NOP, // 5x
    RTS, ADC, NOP, NOP, NOP, ADC, ROR, NOP, PLA, ADC, ROR, NOP, JMP, ADC, ROR, NOP, // 6x
    BVS, ADC, NOP, NOP, NOP, ADC, ROR, NOP, SEI, ADC, NOP, NOP, NOP, ADC, ROR, NOP, // 7x
    NOP, STA, NOP, NOP, STY, STA, STX, NOP, DEY, NOP, TXA, NOP, STY, STA, STX, NOP, // 8x
    BCC, STA, NOP, NOP, STY, STA, STX, NOP, TYA, STA, TXS, NOP, NOP, STA, NOP, NOP, // 9x
    LDY, LDA, LDX, NOP, LDY, LDA, LDX, NOP, TAY, LDA, TAX, NOP, LDY, LDA, LDX, NOP, // Ax
    BCS, LDA, NOP, NOP, LDY, LDA, LDX, NOP, CLV, LDA, TSX, NOP, LDY, LDA, LDX, NOP, // Bx
    CPY, CMP, NOP, NOP, CPY, CMP, DEC, NOP, INY, CMP, DEX, NOP, CPY, CMP, DEC, NOP, // Cx
    BNE, CMP, NOP, NOP, NOP, CMP, DEC, NOP, CLD, CMP, NOP, NOP, NOP, CMP, DEC, NOP, // Dx
    CPX, SBC, NOP, NOP, CPX, SBC, INC, NOP, INX, SBC, NOP, NOP, CPX, SBC, INC, NOP, // Ex
    BEQ, SBC, NOP, NOP, NOP, SBC, INC, NOP, SED, SBC, NOP, NOP, NOP, SBC, INC, NOP, // Fx
};

// I just don't want to type out the literal function name every time
#define INST_HANDLER(x) \
    mos6502_handle_##x

static mos6502_instruction_handler instruction_handlers[] = {
    INST_HANDLER(adc),
    INST_HANDLER(and),
    INST_HANDLER(asl),
    INST_HANDLER(bcc),
    INST_HANDLER(bcs),
    INST_HANDLER(beq),
    INST_HANDLER(bit),
    INST_HANDLER(bmi),
    INST_HANDLER(bne),
    INST_HANDLER(bpl),
    INST_HANDLER(brk),
    INST_HANDLER(bvc),
    INST_HANDLER(bvs),
    INST_HANDLER(clc),
    INST_HANDLER(cld),
    INST_HANDLER(cli),
    INST_HANDLER(clv),
    INST_HANDLER(cmp),
    INST_HANDLER(cpx),
    INST_HANDLER(cpy),
    INST_HANDLER(dec),
    INST_HANDLER(dex),
    INST_HANDLER(dey),
    INST_HANDLER(eor),
    INST_HANDLER(inc),
    INST_HANDLER(inx),
    INST_HANDLER(iny),
    INST_HANDLER(jmp),
    INST_HANDLER(jsr),
    INST_HANDLER(lda),
    INST_HANDLER(ldx),
    INST_HANDLER(ldy),
    INST_HANDLER(lsr),
    INST_HANDLER(nop),
    INST_HANDLER(ora),
    INST_HANDLER(pha),
    INST_HANDLER(php),
    INST_HANDLER(pla),
    INST_HANDLER(plp),
    INST_HANDLER(rol),
    INST_HANDLER(ror),
    INST_HANDLER(rti),
    INST_HANDLER(rts),
    INST_HANDLER(sbc),
    INST_HANDLER(sec),
    INST_HANDLER(sed),
    INST_HANDLER(sei),
    INST_HANDLER(sta),
    INST_HANDLER(stx),
    INST_HANDLER(sty),
    INST_HANDLER(tax),
    INST_HANDLER(tay),
    INST_HANDLER(tsx),
    INST_HANDLER(txa),
    INST_HANDLER(txs),
    INST_HANDLER(tya),
};

static int cycles[] = {
//   00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F
      7,   6,   0,   0,   0,   3,   5,   0,   3,   2,   2,   0,   0,   4,   6,   0, // 0x
      2,   5,   0,   0,   0,   4,   6,   0,   2,   4,   0,   0,   0,   4,   7,   0, // 1x
      6,   6,   0,   0,   3,   3,   5,   0,   4,   2,   2,   0,   4,   4,   6,   0, // 2x
      2,   5,   0,   0,   0,   4,   6,   0,   2,   4,   0,   0,   0,   4,   7,   0, // 3x
      6,   6,   0,   0,   0,   3,   5,   0,   3,   2,   2,   0,   3,   4,   6,   0, // 4x
      2,   5,   0,   0,   0,   4,   6,   0,   2,   4,   0,   0,   0,   4,   7,   0, // 5x
      6,   6,   0,   0,   0,   3,   5,   0,   4,   2,   2,   0,   5,   4,   6,   0, // 6x
      2,   5,   0,   0,   0,   4,   6,   0,   2,   4,   0,   0,   0,   4,   7,   0, // 7x
      0,   6,   0,   0,   3,   3,   3,   0,   2,   0,   2,   0,   4,   4,   4,   0, // 8x
      2,   6,   0,   0,   4,   4,   4,   0,   2,   5,   2,   0,   0,   5,   0,   0, // 9x
      2,   6,   2,   0,   3,   3,   3,   0,   2,   2,   2,   0,   4,   4,   4,   0, // Ax
      2,   5,   0,   0,   4,   4,   4,   0,   2,   4,   2,   0,   4,   4,   4,   0, // Bx
      2,   6,   0,   0,   3,   3,   5,   0,   2,   2,   2,   0,   4,   4,   3,   0, // Cx
      2,   5,   0,   0,   0,   4,   6,   0,   2,   4,   0,   0,   0,   4,   7,   0, // Dx
      2,   6,   0,   0,   3,   3,   5,   0,   2,   2,   2,   0,   4,   4,   6,   0, // Ex
      2,   5,   0,   0,   0,   4,   6,   0,   2,   4,   0,   0,   0,   4,   7,   0, // Fx
};

/*
 * Build a new mos6502 struct object, and also build the memory contents
 * used therein. All registers should be zeroed out.
 */
mos6502 *
mos6502_create()
{
    mos6502 *cpu;
   
    cpu = malloc(sizeof(mos6502));
    if (cpu == NULL) {
        log_critical("Not enough memory to allocate mos6502");
        exit(1);
    }

    cpu->memory = vm_segment_create(MOS6502_MEMSIZE);

    cpu->last_addr = 0;
    cpu->cycles = 0;
    cpu->PC = 0;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->P = 0;
    cpu->S = 0;

    return cpu;
}

/*
 * Free the memory consumed by the mos6502 struct.
 */
void
mos6502_free(mos6502 *cpu)
{
    vm_segment_free(cpu->memory);
    free(cpu);
}

/*
 * Return the next byte from the PC register position, and increment the
 * PC register.
 */
vm_8bit
mos6502_next_byte(mos6502 *cpu)
{
    vm_8bit byte;

    byte = vm_segment_get(cpu->memory, cpu->PC);
    cpu->PC++;

    return byte;
}

void
mos6502_push_stack(mos6502 *cpu, vm_16bit addr)
{
    // First we need to set the hi byte, by shifting the address right 8
    // positions and using the base offset of the S register.
    vm_segment_set(cpu->memory, 0x0100 + cpu->S, addr >> 8);

    // Next we must record the lo byte, this time by using a bitmask to
    // capture just the low end of addr, but recording it in S + 1.
    vm_segment_set(cpu->memory, 0x0100 + cpu->S + 1, addr & 0xFF);

    // And finally we need to increment S by 2 (since we've used two
    // bytes in the stack).
    cpu->S += 2;
}

vm_16bit
mos6502_pop_stack(mos6502 *cpu)
{
    // The first thing we want to do here is to decrement S by 2, since
    // the value we want to return is two positions back.
    cpu->S -= 2;

    // We need to use a bitwise-or operation to combine the hi and lo
    // bytes we retrieve from the stack into the actual position we
    // would use for the PC register.
    return
        (vm_segment_get(cpu->memory, 0x0100 + cpu->S) << 8) |
        vm_segment_get(cpu->memory, 0x0100 + cpu->S + 1);
}

void
mos6502_set_status(mos6502 *cpu, vm_8bit status)
{
    cpu->P = status;
}

void
mos6502_modify_status(mos6502 *cpu, vm_8bit status, vm_8bit oper)
{
    if (status & NEGATIVE) {
        cpu->P &= ~NEGATIVE;
        if (oper & 0x80) {
            cpu->P |= NEGATIVE;
        }
    }

    if (status & OVERFLOW) {
        cpu->P &= ~OVERFLOW;
        if (oper & OVERFLOW) {
            cpu->P |= OVERFLOW;
        }
    }

    if (status & CARRY) {
        cpu->P &= ~CARRY;
        if (oper > 0) {
            cpu->P |= CARRY;
        }
    }

    if (status & ZERO) {
        cpu->P &= ~ZERO;
        if (oper == 0) {
            cpu->P |= ZERO;
        }
    }
}

int
mos6502_instruction(vm_8bit opcode)
{
    return instructions[opcode];
}

int
mos6502_cycles(mos6502 *cpu, vm_8bit opcode)
{
    // In some contexts, we may need to return an additional cycle.
    int modif = 0;

    int addr_mode;
    int lo_addr;

    addr_mode = mos6502_addr_mode(opcode);

    // Mainly we care about the lo byte of the last effective address
    lo_addr = cpu->last_addr & 0xFF;

    // Ok, here's the deal: if you are using an address mode that uses
    // any of the index registers, you need to return an additional
    // cycle if the lo byte of the address plus that index would cross a
    // memory page boundary
    switch (addr_mode) {
        case ABX:
            if (lo_addr + cpu->X > 255) {
                modif = 1;
            }
            break;

        case ABY:
        case INY:
            if (lo_addr + cpu->Y > 255) {
                modif = 1;
            }
            break;

        default:
            break;
    }

    return cycles[opcode] + modif;
}

/*
 * Here we intend to return the proper resolver function for any given
 * instruction.
 */
mos6502_instruction_handler 
mos6502_get_instruction_handler(vm_8bit opcode)
{
    return instruction_handlers[mos6502_instruction(opcode)];
}
