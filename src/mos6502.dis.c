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
mos6502_dis_instruction(FILE *stream, vm_8bit opcode)
{
    int inst_code;

    inst_code = mos6502_instruction(opcode);
    
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
mos6502_dis_expected_bytes(vm_8bit opcode)
{
    int addr_mode = mos6502_addr_mode(opcode);

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
