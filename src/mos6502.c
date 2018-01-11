/*
 * mos6502.c
 *
 * These functions are kind of the "top-level", if you will, for the MOS
 * 6502 processor. You can create the processor struct, operate on the
 * stack, etc.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "mos6502.h"
#include "mos6502.dis.h"

// All of our address modes, instructions, etc. are defined here.
#include "mos6502.enums.h"

/*
 * This is a table which defines what instruction each opcode is mapped
 * to. All possible (256) values are defined here. You will note many
 * cases where we use NOP where opcodes are not _technically_ defined;
 * this may or may not be the best behavior. It's quite possible we should
 * instead crash the program when we stumble upon such malformed opcodes
 */
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

/*
 * A small convenience for defining instruction handlers below.
 */
#define INST_HANDLER(x) \
    mos6502_handle_##x

/*
 * Here's another table, this time mapping instruction codes to
 * instruction handler functions. They are listed in the order defined
 * in the instruction enum (in mos6502.enums.h).
 */
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

/*
 * Here we have a table that maps opcodes to the number of cycles each
 * should cost. In cases where no opcode is defined, we set the number
 * of cycles to zero.
 */
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
mos6502_create(vm_segment *rmem, vm_segment *wmem)
{
    mos6502 *cpu;
   
    cpu = malloc(sizeof(mos6502));
    if (cpu == NULL) {
        log_critical("Not enough memory to allocate mos6502");
        exit(1);
    }

    cpu->rmem = rmem;
    cpu->wmem = wmem;

    cpu->last_addr = 0;
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
    // Note we do not free rmem or wmem; we consider this to be the
    // responsibility of the caller that passed us those values.
    free(cpu);
}

/*
 * Push a _16-bit_ number to the stack. Generally speaking, only
 * addresses are pushed to the stack, such that would be contained in
 * the PC register (which is 16-bit).
 *
 * The stack is contained within a single page of memory, so you would
 * be right in observing that the stack can contain at most 128, not
 * 256, addresses.
 */
void
mos6502_push_stack(mos6502 *cpu, vm_16bit addr)
{
    // First we need to set the hi byte, by shifting the address right 8
    // positions and using the base offset of the S register.
    mos6502_set(cpu, 0x0100 + cpu->S, addr & 0xff);

    // Next we must record the lo byte, this time by using a bitmask to
    // capture just the low end of addr, but recording it in S + 1.
    mos6502_set(cpu, 0x0100 + cpu->S + 1, addr >> 8);

    // And finally we need to increment S by 2 (since we've used two
    // bytes in the stack).
    cpu->S += 2;
}

/*
 * Pop an address from the stack and return that.
 */
vm_16bit
mos6502_pop_stack(mos6502 *cpu)
{
    // The first thing we want to do here is to decrement S by 2, since
    // the value we want to return is two positions back.
    cpu->S -= 2;

    // We need to use a bitwise-or operation to combine the hi and lo
    // bytes we retrieve from the stack into the actual position we
    // would use for the PC register.
    return mos6502_get16(cpu, 0x0100 + cpu->S);
}

/*
 * Here we set the status register to a given status value, regardless
 * of its past contents. 
 */
void
mos6502_set_status(mos6502 *cpu, vm_8bit status)
{
    cpu->P = status;
}

/*
 * In contrast, the modify_status function will conditionally set the
 * contents of certain bits, based upon the value of the operand. Those
 * bits are the negative, overflow, carry, and zero flags.
 */
void
mos6502_modify_status(mos6502 *cpu, vm_8bit status, vm_8bit oper)
{
    if (status & MOS_NEGATIVE) {
        cpu->P &= ~MOS_NEGATIVE;
        if (oper & 0x80) {
            cpu->P |= MOS_NEGATIVE;
        }
    }

    if (status & MOS_OVERFLOW) {
        cpu->P &= ~MOS_OVERFLOW;
        if (oper & MOS_OVERFLOW) {
            cpu->P |= MOS_OVERFLOW;
        }
    }

    if (status & MOS_CARRY) {
        cpu->P &= ~MOS_CARRY;
        if (oper > 0) {
            cpu->P |= MOS_CARRY;
        }
    }

    if (status & MOS_ZERO) {
        cpu->P &= ~MOS_ZERO;
        if (oper == 0) {
            cpu->P |= MOS_ZERO;
        }
    }
}

/*
 * Return the instruction that is mapped to a given opcode.
 */
int
mos6502_instruction(vm_8bit opcode)
{
    return instructions[opcode];
}

/*
 * Return the number of cycles an opcode may consume. The cpu is a
 * required parameter, because the number of opcodes is conditional upon
 * the effective address of the instruction we're executing.
 */
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

/*
 * This code does the execution step that the 6502 processor would take,
 * from soup to nuts.
 */
void
mos6502_execute(mos6502 *cpu)
{
    vm_8bit opcode, operand = 0;
    int cycles, bytes;
    mos6502_address_resolver resolver;
    mos6502_instruction_handler handler;

    opcode = mos6502_get(cpu, cpu->PC);

    // The disassembler knows how many bytes each operand requires
    // (maybe this code doesn't belong in the disassembler); let's use
    // that to figure out the total number of bytes to skip. We add 1
    // because we need to account for the opcode as well.
    bytes = 1 + mos6502_dis_expected_bytes(mos6502_addr_mode(opcode));

    // First, we need to know how to resolve our effective address and
    // how to execute anything.
    resolver = mos6502_get_address_resolver(opcode);
    handler = mos6502_get_instruction_handler(opcode);

    // The operand is the effective operand, the value that the
    // instruction handler cares about (if it cares about any such
    // value). For example, the operand could be the literal value that
    // you pass into an instruction via immediate mode. As a
    // side-effect, resolver will set the last_addr field in cpu to the
    // effective address where the operand can be found in memory, or
    // zero if that does not apply (such as in immediate mode).
    //
    // Note also that resolver may be NULL, as there may not be any
    // operand for this instruction! If so, we let the default for
    // operand stand, which is zero.
    if (resolver) {
        operand = resolver(cpu);
    }

    // Here's where the magic happens. Whatever the instruction does, it
    // happens in the handler function.
    handler(cpu, operand);

    // This will be the number of cycles we should spend on the
    // instruction. Of course, we can execute instructions pretty
    // quickly in a modern architecture, but a lot of code was written
    // with the idea that certain instructions -- in certain address
    // modes -- were more expensive than others, and you want those
    // programs to feel faster or slower in relation to that.
    cycles = mos6502_cycles(cpu, opcode);

    // If we need to jump, then the handler has to take care of updating
    // PC. If not, then we need to do it. 
    if (!mos6502_would_jump(mos6502_instruction(opcode))) {
        cpu->PC += bytes;
    }

    // FIXME: uh this probably isn't right, but I wanted to do
    // something.
    usleep(cycles * 10000);

    // Ok -- we're done! This wasn't so hard, was it?
    return;
}

/*
 * Return true if the given instruction would require that we jump
 * to somewhere else in the program.
 */
inline bool
mos6502_would_jump(int inst_code)
{
    return 
        inst_code == BCC ||
        inst_code == BCS ||
        inst_code == BEQ ||
        inst_code == BMI ||
        inst_code == BNE ||
        inst_code == BPL ||
        inst_code == BRK ||
        inst_code == BVC ||
        inst_code == BVS ||
        inst_code == JMP ||
        inst_code == JSR ||
        inst_code == RTS ||
        inst_code == RTI;
}

/*
 * Here we copy the segment directly into the cpu memory, to essentially
 * "flash" memory with the contents of another segment.
 */
void
mos6502_flash_memory(mos6502 *cpu, vm_segment *segment)
{
    //vm_segment_copy(cpu, segment, 0, 0, cpu->size - 1);
}

/*
 * This is a _kind_ of factory method, except we're obviously not
 * instantiating an object. Given an address mode, we return the
 * resolver function which will give you the right value (for a given
 * cpu) that an instruction will use.
 */
mos6502_address_resolver
mos6502_get_address_resolver(vm_8bit opcode)
{
    switch (mos6502_addr_mode(opcode)) {
        case ACC: return mos6502_resolve_acc;
        case ABS: return mos6502_resolve_abs;
        case ABX: return mos6502_resolve_abx;
        case ABY: return mos6502_resolve_aby;
        case IMM: return mos6502_resolve_imm;
        case IND: return mos6502_resolve_ind;
        case IDX: return mos6502_resolve_idx;
        case IDY: return mos6502_resolve_idy;
        case REL: return mos6502_resolve_rel;
        case ZPG: return mos6502_resolve_zpg;
        case ZPX: return mos6502_resolve_zpx;
        case ZPY: return mos6502_resolve_zpy;
        case IMP:   // FALLTHRU
        default: break;
    }

    return NULL;
}

inline vm_8bit
mos6502_get(mos6502 *cpu, size_t addr)
{
    return vm_segment_get(cpu->rmem, addr);
}

inline vm_16bit
mos6502_get16(mos6502 *cpu, size_t addr)
{
    return vm_segment_get16(cpu->rmem, addr);
}

inline void
mos6502_set(mos6502 *cpu, size_t addr, vm_8bit value)
{
    vm_segment_set(cpu->wmem, addr, value);
}

inline void
mos6502_set16(mos6502 *cpu, size_t addr, vm_16bit value)
{
    vm_segment_set16(cpu->wmem, addr, value);
}
