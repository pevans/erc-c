/*
 * mos6502.dis.c
 *
 * Disassembly of the mos6502 machine code into an assembly notation.
 */

#include "mos6502.h"
#include "mos6502.enums.h"

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
mos6502_dis_operand(FILE *stream, int addr_mode, vm_16bit value)
{
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
            fprintf(stream, "($%04X)", value);
            break;
        case IDX:
            fprintf(stream, "($%02X,X)", value);
            break;
        case IDY:
            fprintf(stream, "($%02X),Y", value);
            break;
        case REL:
            // FIXME: we need some kind of table of jumps and branches
            // we make, so that we can come up with some labels to use.
            fprintf(stream, "(REL)");
            break;
        case ZPG:
            fprintf(stream, "$%02X", value);
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
 * Scan the segment (with a given address) and write the opcode at that
 * point to the given file stream. This function will also write an
 * operand to the file stream if one is warranted. We return the number
 * of bytes consumed by scanning past the opcode and/or operand.
 */
int
mos6502_dis_scan(FILE *stream, vm_segment *segment, int address)
{
    vm_8bit opcode;
    vm_16bit operand;
    int addr_mode;
    int expected;

    // The next byte is assumed to be the opcode we work with.
    opcode = vm_segment_get(segment, address);

    // And given that opcode, we need to see how many bytes large our
    // operand will be.
    addr_mode = mos6502_addr_mode(opcode);
    expected = mos6502_dis_expected_bytes(addr_mode);

    // The operand itself defaults to zero... in cases where this
    // doesn't change, the instruction related to the opcode will
    // probably not even use it.
    operand = 0;

    // And we need to skip ahead of the opcode.
    address++;

    switch (expected) {
        case 2:
            // If we have a two-byte operand, then the first byte is the
            // MSB and our operand will need to shift it 8 positions to
            // the left before it can be OR'd.
            operand |= vm_segment_get(segment, address) << 8;
            address++;

            // Note we fall through here to the next case...

        case 1:
            // If it's a one-byte operand, then this byte should occupy
            // the LSB space which simply requires we OR the byte
            // directly in. If this is part of a two-byte operand, then
            // the same logic still applies.
            operand |= vm_segment_get(segment, address);
            address++;

            // And, in any other case (e.g. 0), we are done; we don't
            // read anything, and we leave the operand as it is.
        default:
            break;
    }

    // Let's print out to the stream what we have so far. First, we
    // indent by four spaces.
    fprintf(stream, "    ");

    // Print out the instruction code that our opcode represents.
    mos6502_dis_instruction(stream, mos6502_instruction(opcode));

    // Let's "tab" over; each instruction code is 3 characters, so let's
    // move over 5 spaces (4 spaces indent + 1, just to keep everything
    // aligned by 4-character boundaries).
    fprintf(stream, "     ");

    // Print out the operand given the proper address mode.
    mos6502_dis_operand(stream, mos6502_addr_mode(opcode), operand);

    // And let's terminate the line.
    fprintf(stream, "\n");

    // The expected number of bytes here is for the operand, but we need
    // to add one for the opcode to return the true number that this
    // opcode sequence would consume.
    return expected + 1;
}
