/*
 * mos6502.dis.c
 *
 * Disassembly of the mos6502 machine code into an assembly notation.
 */

#include <stdbool.h>

#include "mos6502.h"
#include "mos6502.dis.h"
#include "mos6502.enums.h"

static vm_8bit jump_table[MOS6502_MEMSIZE];

static char *instruction_strings[] = {
    "ADC",
    "AND",
    "ASL",
    "BCC",
    "BCS",
    "BEQ",
    "BIT",
    "BMI",
    "BNE",
    "BPL",
    "BRK",
    "BVC",
    "BVS",
    "CLC",
    "CLD",
    "CLI",
    "CLV",
    "CMP",
    "CPX",
    "CPY",
    "DEC",
    "DEX",
    "DEY",
    "EOR",
    "INC",
    "INX",
    "INY",
    "JMP",
    "JSR",
    "LDA",
    "LDX",
    "LDY",
    "LSR",
    "NOP",
    "ORA",
    "PHA",
    "PHP",
    "PLA",
    "PLP",
    "ROL",
    "ROR",
    "RTI",
    "RTS",
    "SBC",
    "SEC",
    "SED",
    "SEI",
    "STA",
    "STX",
    "STY",
    "TAX",
    "TAY",
    "TSX",
    "TXA",
    "TXS",
    "TYA",
};

/*
 * Given a stream, address mode and 16-bit value, print the value out in
 * the form that is expected given the address mode. The value is not
 * necessarily going to truly be 16-bit; most address modes use one
 * 8-bit operand. But we can contain all possible values with the 16-bit
 * type.
 */
void
mos6502_dis_operand(mos6502 *cpu,
                    FILE *stream, 
                    int address, 
                    int addr_mode, 
                    vm_16bit value)
{
    int rel_address;
    int ind_address;

    switch (addr_mode) {
        case ACC:
            break;
        case ABS:
            fprintf(stream, "$%04X", value);
            break;
        case ABX:
            fprintf(stream, "$%04X,X", value);
            break;
        case ABY:
            fprintf(stream, "$%04X,Y", value);
            break;
        case IMM:
            fprintf(stream, "#$%02X", value);
            break;
        case IMP:
            break;
        case IND:
            ind_address = vm_segment_get(cpu->memory, value + 1) << 8;
            ind_address |= vm_segment_get(cpu->memory, value);
            if (jump_table[ind_address]) {
                mos6502_dis_label(stream, ind_address);
            } else {
                fprintf(stream, "($%04X)", value);
            }
            break;
        case IDX:
            fprintf(stream, "($%02X,X)", value);
            break;
        case IDY:
            fprintf(stream, "($%02X),Y", value);
            break;
        case REL:
            rel_address = address + value;
            if (value > 127) {
                rel_address -= 256;
            }

            mos6502_dis_label(stream, rel_address);
            break;
        case ZPG:
            // We add a couple of spaces here to help our output
            // comments line up.
            fprintf(stream, "$%02X  ", value);
            break;
        case ZPX:
            fprintf(stream, "$%02X,X", value);
            break;
        case ZPY:
            fprintf(stream, "$%02X,Y", value);
            break;
    }
}

/*
 * This function will write to the stream the instruction that the given
 * opcode maps to.
 */
void
mos6502_dis_instruction(FILE *stream, int inst_code)
{
    // Arguably this could or should be done as fputs(), which is
    // presumably a simpler output method. But, since we use fprintf()
    // in other places, I think we should continue to do so. Further, we
    // use a simple format string (%s) to avoid the linter's complaints
    // about potential security issues.
    fprintf(stream, "%s", instruction_strings[inst_code]);
}

/*
 * This function returns the number of bytes that the given opcode is
 * expecting to work with. For instance, if the opcode is in absolute
 * address mode, then we will need to read the next two bytes in the
 * stream to compose a full 16-bit address to work with. If our opcode
 * is in immediate mode, then we only need to read one byte. Many
 * opcodes will read no bytes at all from the stream (in which we return
 * zero).
 */
int
mos6502_dis_expected_bytes(int addr_mode)
{
    switch (addr_mode) {
        // These are 16-bit operands, because they work with absolute
        // addresses in memory.
        case ABS:
        case ABY:
        case ABX:
        case IND:
            return 2;

        // These are the 8-bit operand address modes.
        case IMM:
        case IDX:
        case IDY:
        case REL:
        case ZPG:
        case ZPX:
        case ZPY:
            return 1;

        // These two address modes have implied arguments; ACC is
        // the accumulator, and IMP basically means it operates on
        // some specific (presumably obvious) thing and no operand
        // is necessary.
        case ACC:
        case IMP:
            return 0;
    }

    // I don't know how we got here, outside of foul magicks and cruel
    // trickery. Let's fearfully return zero!
    return 0;
}

/*
 * Scan memory (with a given address) and write the opcode at that
 * point to the given file stream. This function will also write an
 * operand to the file stream if one is warranted. We return the number
 * of bytes consumed by scanning past the opcode and/or operand.
 */
int
mos6502_dis_opcode(mos6502 *cpu, FILE *stream, int address)
{
    vm_8bit opcode;
    vm_16bit operand;
    int addr_mode;
    int inst_code;
    int expected;

    // The next byte is assumed to be the opcode we work with.
    opcode = vm_segment_get(cpu->memory, address);

    // And given that opcode, we need to see how many bytes large our
    // operand will be.
    addr_mode = mos6502_addr_mode(opcode);
    expected = mos6502_dis_expected_bytes(addr_mode);

    // The specific instruction we mean to execute
    inst_code = mos6502_instruction(opcode);

    // The operand itself defaults to zero... in cases where this
    // doesn't change, the instruction related to the opcode will
    // probably not even use it.
    operand = 0;

    // And we need to skip ahead of the opcode.
    address++;

    switch (expected) {
        case 2:
            // Remember that the 6502 is little-endian, so the operand
            // needs to be retrieved with the LSB first and the MSB
            // second.
            operand |= vm_segment_get(cpu->memory, address++);
            operand |= vm_segment_get(cpu->memory, address++) << 8;
            break;

        case 1:
            operand |= vm_segment_get(cpu->memory, address++);
            break;

            // And, in any other case (e.g. 0), we are done; we don't
            // read anything, and we leave the operand as it is.
        default:
            break;
    }

    // If the stream is NULL, we're doing some kind of lookahead.
    // Furthermore, if this is an instruction that would switch control
    // to a different spot in the program, then let's label this in the
    // jump table.
    if (stream == NULL && mos6502_would_jump(inst_code)) {
        mos6502_dis_jump_label(cpu, operand, address, addr_mode);
    }

    // It's totally possible that we are not expected to print out the
    // contents of our inspection of the opcode. (For example, we may
    // just want to set the jump table in a lookahead operation.)
    if (stream) {
        // Hey! We might have a label at this position in the code. If
        // so, let's print out the label.
        if (jump_table[address]) {
            // This will print out just the label itself.
            mos6502_dis_label(stream, address);

            // But to actually define the label, we need a colon to
            // complete the notation. (We don't _need_ a newline, but it
            // looks nicer to my arbitrary sensibilities. Don't @ me!)
            fprintf(stream, ":\n");
        }

        // Let's print out to the stream what we have so far. First, we
        // indent by four spaces.
        fprintf(stream, "    ");

        // Print out the instruction code that our opcode represents.
        mos6502_dis_instruction(stream, inst_code);

        // Let's "tab" over; each instruction code is 3 characters, so let's
        // move over 5 spaces (4 spaces indent + 1, just to keep everything
        // aligned by 4-character boundaries).
        fprintf(stream, "     ");

        if (expected) {
            // Print out the operand given the proper address mode.
            mos6502_dis_operand(cpu, stream, address, addr_mode, operand);
        } else {
            // Print out a tab to get a consistent look in our
            // disassembled code (e.g. to take up the space that an
            // operand would otherwise occupy).
            fprintf(stream, "\t");
        }

        // Here we just want to show a few pieces of information; one,
        // what the PC was at the point of this opcode sequence; two,
        // the opcode;
        fprintf(stream, "\t; pc=$%02x%02x: %02x", 
                cpu->PC >> 8, cpu->PC & 0xff, opcode);

        // And three, the operand, if any. Remembering that the operand
        // should be shown in little-endian order.
        if (expected == 2) {
            fprintf(stream, " %02x %02x", operand & 0xff, operand >> 8);
        } else if (expected == 1) {
            fprintf(stream, " %02x", operand & 0xff);
        }

        // And let's terminate the line.
        fprintf(stream, "\n");
    }

    // The expected number of bytes here is for the operand, but we need
    // to add one for the opcode to return the true number that this
    // opcode sequence would consume.
    return expected + 1;
}

/*
 * Scan the CPU memory, from a given position until a given end, and
 * print the results into a given file stream.
 */
void
mos6502_dis_scan(mos6502 *cpu, FILE *stream, int pos, int end)
{
    while (pos < end) {
        pos += mos6502_dis_opcode(cpu, stream, pos);
    }
}

/*
 * Associate a label with a given address or operand, depending on the
 * address mode. For example, with REL, the jump label will be based on
 * the address but added to or subtracted with the operand. Whereas in
 * IND, the address is wholly dependent on the operand.
 */
void
mos6502_dis_jump_label(mos6502 *cpu, 
                       vm_16bit operand, 
                       int address, 
                       int addr_mode)
{
    int jump_loc;

    switch (addr_mode) {
        // With indirect address mode, the address we want to jump to is
        // not the literal operand, but a 16-bit address that is
        // _pointed to_ by the address represented by the operand. Think
        // of the operand as a kind of double pointer, or just re-watch
        // Inception.
        case IND:
            jump_loc = vm_segment_get(cpu->memory, operand) << 8;
            jump_loc |= vm_segment_get(cpu->memory, operand + 1);
            break;

        // In relative address mode, the jump location will be a
        // number -- well -- relative to the address. If the 8th bit
        // of the operand is 1, then we treat the number as a
        // negative; otherwise, positive or zero.
        case REL:
            jump_loc = address + operand;

            if (operand > 127) {
                jump_loc -= 256;
            }
            break;

        default:
            return;
    }

    jump_table[jump_loc] = 1;
}

/*
 * Print out the form of our label to the given file stream. This is
 * fairly dumb; it'll print out whatever address you give to it.
 */
inline void
mos6502_dis_label(FILE *stream, int address)
{
    fprintf(stream, "ADDR_%x", address);
}

/*
 * Remove the previously-set label in the jump table for a given
 * address.
 */
inline void
mos6502_dis_jump_unlabel(int address)
{
    jump_table[address] = 0;
}

/*
 * Return true if the given address has a jump label associated with it.
 */
inline bool
mos6502_dis_is_jump_label(int address)
{
    return jump_table[address] == 1;
}
