/*
 * mos6502.dis.c
 *
 * Disassembly of the mos6502 machine code into an assembly notation. I
 * should note that there is no formal grammar (that I know of!) for
 * 6502 assembly--just an informal notation that is de-facto supported
 * by one assembler or another. The general format we use is as follows:
 *
 * LABEL:
 *     INS    $OPER       ; comment
 *
 * Where LABEL is a--well, a label; INS is an instruction; $OPER is a
 * hexadecimal number; and a semicolon denotes a comment follows until
 * the end of the line.
 *
 * You will find a number of variants of `$OPER`, as the assembly
 * notation uses those variants to denote a specific kind of address
 * mode. `$OPER` is absolute mode; `$OP` (just two hex digits) is
 * zero-page mode; `$(OP),Y` is indirect-indexed mode; etc. (Please
 * refer to mos6502.addr.c for more details on those modes!)
 *
 * The code here generally pushes disassembled notation into FILE stream
 * objects. If you need them in a string, for instance, you can mess
 * with `setvbuf()` (as we indeed do in our unit-testing code!).
 */

#include <stdbool.h>

#include "mos6502.h"
#include "mos6502.dis.h"
#include "mos6502.enums.h"

static char s_bytes[9],
            s_inst[4],
            s_operand[11];

static char *instruction_strings[] = {
    "ADC",
    "AND",
    "ASL",
    "BAD",
    "BCC",
    "BCS",
    "BEQ",
    "BIT",
    "BIM",
    "BMI",
    "BNE",
    "BPL",
    "BRA",
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
    "NP2",
    "NP3",
    "ORA",
    "PHA",
    "PHP",
    "PHX",
    "PHY",
    "PLA",
    "PLP",
    "PLX",
    "PLY",
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
    "STZ",
    "TAX",
    "TAY",
    "TRB",
    "TSB",
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
                    char *str,
                    int len, 
                    int address, 
                    int addr_mode, 
                    vm_16bit value)
{
    int rel_address;
    vm_8bit eff_value = 0;
    mos6502_address_resolver resolv;

    resolv = mos6502_get_address_resolver(addr_mode);
    if (resolv) {
        eff_value = resolv(cpu);
    }

    switch (addr_mode) {
        case ACC:
            break;
        case ABS:
            snprintf(str, len, "$%04X", value);
            break;
        case ABX:
            snprintf(str, len, "$%04X,X", value);
            break;
        case ABY:
            snprintf(str, len, "$%04X,Y", value);
            break;
        case IMM:
            snprintf(str, len, "#$%02X", value);
            break;
        case IMP:
            snprintf(str, len, "");
            break;
        case IND:
            snprintf(str, len, "($%04X)", value);
            break;
        case IDX:
            snprintf(str, len, "($%02X,X)", value);
            break;
        case IDY:
            snprintf(str, len, "($%02X),Y", value);
            break;
        case REL:
            rel_address = address + value;
            if (value > 127) {
                rel_address -= 256;
            }

            snprintf(str, len, "<%04x>", rel_address);
            break;
        case ZPG:
            // We add a couple of spaces here to help our output
            // comments line up.
            snprintf(str, len, "$%02X", value);
            break;
        case ZPX:
            snprintf(str, len, "$%02X,X", value);
            break;
        case ZPY:
            snprintf(str, len, "$%02X,Y", value);
            break;
    }
}

/*
 * This function will write to the stream the instruction that the given
 * opcode maps to.
 */
void
mos6502_dis_instruction(char *str, int len, int inst_code)
{
    // Arguably this could or should be done as fputs(), which is
    // presumably a simpler output method. But, since we use snprintf()
    // in other places, I think we should continue to do so. Further, we
    // use a simple format string (%s) to avoid the linter's complaints
    // about potential security issues.
    snprintf(str, len, "%s", instruction_strings[inst_code]);
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
        // This is kind of not a real address mode? We use it to tell
        // the code to skip three bytes for opcodes that use it.
        case BY3:
            return 3;

        // These are 16-bit operands, because they work with absolute
        // addresses in memory.
        case ABS:
        case ABY:
        case ABX:
        case BY2:   // (also not a real address mode!)
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
    opcode = mos6502_get(cpu, address);

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
            operand |= mos6502_get(cpu, address++);
            operand |= mos6502_get(cpu, address++) << 8;
            break;

        case 1:
            operand |= mos6502_get(cpu, address++);
            break;

            // And, in any other case (e.g. 0), we are done; we don't
            // read anything, and we leave the operand as it is.
        default:
            break;
    }

    // It's totally possible that we are not expected to print out the
    // contents of our inspection of the opcode. (For example, we may
    // just want to set the jump table in a lookahead operation.)
    if (stream) {
        // Print out the instruction code that our opcode represents.
        mos6502_dis_instruction(s_inst, sizeof(s_inst), inst_code);

        if (expected) {
            // Print out the operand given the proper address mode.
            mos6502_dis_operand(cpu, s_operand, sizeof(s_operand), 
                                address, addr_mode, operand);
        }

        // And three, the operand, if any. Remembering that the operand
        // should be shown in little-endian order.
        if (expected == 2) {
            snprintf(s_bytes, sizeof(s_bytes) - 1, "%02X %02X %02X", 
                     opcode, operand & 0xff, operand >> 8);
        } else if (expected == 1) {
            snprintf(s_bytes, sizeof(s_bytes) - 1, "%02X %02X", 
                     opcode, operand & 0xff);
        } else {
            snprintf(s_bytes, sizeof(s_bytes) - 1, "%02X", opcode);
        }
    }

    fprintf(stream, "%04X:%-9s%20s   %s\n",
            cpu->PC, s_bytes, s_inst, s_operand);

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
