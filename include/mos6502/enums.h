/*
 * mos6502.enums.h
 *   Enums and other symbols for use with the mos 6502
 *
 * We have separated the definitions of address mode types, instruction
 * types, etc. into their own file so that we can include it in our main
 * source file, as well as from our unit test suite, without necessarily
 * adding them to the global namespace throughout the application.
 */

#ifndef _MOS6502_ENUMS_H_
#define _MOS6502_ENUMS_H_

/*
 * This defines all of the flags that are possible within the status (P)
 * register. Note that there is intentionally _no_ definition for the
 * 6th bit.
 */
enum status_flags {
    MOS_CARRY = 1,
    MOS_ZERO = 2,
    MOS_INTERRUPT = 4,
    MOS_DECIMAL = 8,
    MOS_BREAK = 16,
    MOS_OVERFLOW = 64,
    MOS_NEGATIVE = 128,
};

#define MOS_NVZ (MOS_NEGATIVE | MOS_OVERFLOW | MOS_ZERO)
#define MOS_NVZC (MOS_NEGATIVE | MOS_OVERFLOW | MOS_ZERO | MOS_CARRY)
#define MOS_NZ (MOS_NEGATIVE | MOS_ZERO)
#define MOS_NZC (MOS_NEGATIVE | MOS_ZERO | MOS_CARRY)
#define MOS_ZC (MOS_ZERO | MOS_CARRY)

#define MOS_STATUS_DEFAULT (MOS_NEGATIVE | MOS_OVERFLOW | \
                            MOS_INTERRUPT | MOS_ZERO | MOS_CARRY)

/*
 * Here we define the various address modes that are possible. These do
 * not map to any significant numbers that are documented for the 6502
 * processor; the position of these symbols don't really matter, and are
 * generally (except for `NOA`, no address mode) in alphabetical order.
 */
enum addr_mode {
    NOA,    // no address mode
    ACC,    // accumulator
    ABS,    // absolute
    ABX,    // absolute x-index
    ABY,    // absolute y-index
    BY2,    // Consume 2 bytes (for NP2)
    BY3,    // Consume 3 bytes (for NP3)
    IMM,    // immediate
    IMP,    // implied
    IND,    // indirect
    IDX,    // x-index indirect
    IDY,    // indirect y-index
    REL,    // relative
    ZPG,    // zero page
    ZPX,    // zero page x-index
    ZPY,    // zero page y-index
};

/*
 * These define the various instructions as enum symbols; again, like
 * for address modes, the values of these enums are not actually
 * significant to the 6502 processor, and are only useful to we, the
 * programmers.
 */
enum instruction {
    ADC,    // ADd with Carry
    AND,    // bitwise AND
    ASL,    // Arithmetic Shift Left
    BAD,    // bad instruction
    BCC,    // Branch on Carry Clear
    BCS,    // Branch on Carry Set
    BEQ,    // Branch on EQual to zero
    BIT,    // BIT test
    BIM,    // BIt test (imMediate mode) (* not a real instruction in the processor; just used by us)
    BMI,    // Branch on MInus 
    BNE,    // Branch on Not Equal to zero
    BPL,    // Branch on PLus
    BRA,    // BRanch Always
    BRK,    // BReaK (interrupt)
    BVC,    // Branch on oVerflow Clear
    BVS,    // Branch on oVerflow Set
    CLC,    // CLear Carry
    CLD,    // CLear Decimal
    CLI,    // CLear Interrupt disable
    CLV,    // CLear oVerflow
    CMP,    // CoMPare
    CPX,    // ComPare with X register
    CPY,    // ComPare with Y register
    DEC,    // DECrement
    DEX,    // DEcrement X
    DEY,    // DEcrement Y
    EOR,    // Exclusive OR
    INC,    // INCrement
    INX,    // INcrement X
    INY,    // INcrement Y
    JMP,    // JuMP
    JSR,    // Jump to SubRoutine
    LDA,    // LoaD Accumulator
    LDX,    // LoaD X
    LDY,    // LoaD Y
    LSR,    // Logical Shift Right
    NOP,    // NO oPeration
    NP2,    // No oPeration (2 bytes consumed)
    NP3,    // No oPeration (3 bytes consumed)
    ORA,    // OR with Accumulator
    PHA,    // PusH Accumulator
    PHP,    // PusH Predicate register
    PHX,    // PusH X register
    PHY,    // PusH Y register
    PLA,    // PulL Accumulator
    PLP,    // PulL Predicate register
    PLX,    // PulL X register
    PLY,    // PulL Y register
    ROL,    // ROtate Left
    ROR,    // ROtate Right
    RTI,    // ReTurn from Interrupt
    RTS,    // ReTurn from Subroutine
    SBC,    // SuBtract with Carry
    SEC,    // SEt Carry
    SED,    // SEt Decimal
    SEI,    // SEt Interrupt disable
    STA,    // STore Accumulator
    STX,    // STore X
    STY,    // STore Y
    STZ,    // STore Zero
    TAX,    // Transfer Accumulator to X
    TAY,    // Transfer Accumulator to Y
    TRB,    // Test and Reset Bits
    TSB,    // Test and Set Bits
    TSX,    // Transfer Stack register to X
    TXA,    // Transfer X to Accumulator
    TXS,    // Transfer X to Stack register
    TYA,    // Transfer Y to Accumulator
};

#endif
