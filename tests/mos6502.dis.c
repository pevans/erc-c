#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <unistd.h>

#include "mos6502.dis.h"
#include "mos6502.enums.h"

#define TMPFILE "/dev/null"

static char buf[256];
static FILE *stream = NULL;
static FILE *input = NULL;

static void
setup()
{
    stream = fopen(TMPFILE, "w");
    if (stream == NULL) {
        perror("Could not open temporary file for mos6502 disassembly tests");
    }

    setvbuf(stream, buf, _IOFBF, 256);

    input = fopen(TMPFILE, "r");
    if (input == NULL) {
        perror("Could not open temporary file for mos6502 disassembly tests");
    }

    setvbuf(stream, buf, _IOFBF, 256);
}

static void
teardown()
{
    fclose(stream);
}

static void
assert_buf(const char *str)
{
    rewind(stream);
    cr_assert_str_eq(buf, str);
    memset(buf, 0, sizeof(buf));
}

TestSuite(mos6502_dis, .init = setup, .fini = teardown);

Test(mos6502_dis, operand)
{
    mos6502_dis_operand(stream, ABS, 0x1234);
    assert_buf("$1234");
    mos6502_dis_operand(stream, ABX, 0x1234);
    assert_buf("$1234,X");
    mos6502_dis_operand(stream, ABY, 0x1234);
    assert_buf("$1234,Y");
    mos6502_dis_operand(stream, IMM, 0x12);
    assert_buf("#$12");
    mos6502_dis_operand(stream, IND, 0x1234);
    assert_buf("($1234)");
    mos6502_dis_operand(stream, IDX, 0x12);
    assert_buf("($12,X)");
    mos6502_dis_operand(stream, IDY, 0x34);
    assert_buf("($34),Y");
    mos6502_dis_operand(stream, ZPG, 0x34);
    assert_buf("$34");
    mos6502_dis_operand(stream, ZPX, 0x34);
    assert_buf("$34,X");
    mos6502_dis_operand(stream, ZPY, 0x34);
    assert_buf("$34,Y");
    
    // These should both end up printing nothing
    mos6502_dis_operand(stream, ACC, 0);
    assert_buf("");
    mos6502_dis_operand(stream, IMP, 0);
    assert_buf("");

    // FIXME: this is intentionally "broken" until we can implement a
    // jump table
    mos6502_dis_operand(stream, REL, 0x34);
    assert_buf("(REL)");
}

Test(mos6502_dis, instruction)
{
#define TEST_INST(x) \
    mos6502_dis_instruction(stream, x); \
    assert_buf(#x)

    TEST_INST(ADC);
    TEST_INST(AND);
    TEST_INST(ASL);
    TEST_INST(BCC);
    TEST_INST(BCS);
    TEST_INST(BEQ);
    TEST_INST(BIT);
    TEST_INST(BMI);
    TEST_INST(BNE);
    TEST_INST(BPL);
    TEST_INST(BRK);
    TEST_INST(BVC);
    TEST_INST(BVS);
    TEST_INST(CLC);
    TEST_INST(CLD);
    TEST_INST(CLI);
    TEST_INST(CLV);
    TEST_INST(CMP);
    TEST_INST(CPX);
    TEST_INST(CPY);
    TEST_INST(DEC);
    TEST_INST(DEX);
    TEST_INST(DEY);
    TEST_INST(EOR);
    TEST_INST(INC);
    TEST_INST(INX);
    TEST_INST(INY);
    TEST_INST(JMP);
    TEST_INST(JSR);
    TEST_INST(LDA);
    TEST_INST(LDX);
    TEST_INST(LDY);
    TEST_INST(LSR);
    TEST_INST(NOP);
    TEST_INST(ORA);
    TEST_INST(PHA);
    TEST_INST(PHP);
    TEST_INST(PLA);
    TEST_INST(PLP);
    TEST_INST(ROL);
    TEST_INST(ROR);
    TEST_INST(RTI);
    TEST_INST(RTS);
    TEST_INST(SBC);
    TEST_INST(SEC);
    TEST_INST(SED);
    TEST_INST(SEI);
    TEST_INST(STA);
    TEST_INST(STX);
    TEST_INST(STY);
    TEST_INST(TAX);
    TEST_INST(TAY);
    TEST_INST(TSX);
    TEST_INST(TXA);
    TEST_INST(TXS);
    TEST_INST(TYA);
}

Test(mos6502_dis, expected_bytes)
{
#define TEST_BYTES(x, y) \
    cr_assert_eq(mos6502_dis_expected_bytes(x), y)

    TEST_BYTES(ACC, 0);
    TEST_BYTES(ABS, 2);
    TEST_BYTES(ABX, 2);
    TEST_BYTES(ABY, 2);
    TEST_BYTES(IMM, 1);
    TEST_BYTES(IMP, 0);
    TEST_BYTES(IND, 2);
    TEST_BYTES(IDX, 1);
    TEST_BYTES(IDY, 1);
    TEST_BYTES(REL, 1);
    TEST_BYTES(ZPG, 1);
    TEST_BYTES(ZPX, 1);
    TEST_BYTES(ZPY, 1);
}

Test(mos6502_dis, scan)
{
    vm_segment *segment;
    int bytes;

    segment = vm_segment_create(1000);

    vm_segment_set(segment, 0, 0x29);   // AND (imm)
    vm_segment_set(segment, 1, 0x38);

    bytes = mos6502_dis_scan(stream, segment, 0);
    assert_buf("    AND     #$38\n");
    cr_assert_eq(bytes, 2);
}
