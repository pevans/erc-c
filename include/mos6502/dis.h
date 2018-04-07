#ifndef _MOS6502_DIS_H_
#define _MOS6502_DIS_H_

#include "mos6502/mos6502.h"
#include "vm_bits.h"

extern bool mos6502_dis_is_jump_label(int);
extern int mos6502_dis_expected_bytes(int);
extern int mos6502_dis_opcode(mos6502 *, FILE *, int);
extern void mos6502_dis_instruction(char *, int, int);
extern void mos6502_dis_jump_label(mos6502 *, vm_16bit, int, int);
extern void mos6502_dis_jump_unlabel(int);
extern void mos6502_dis_label(char *, int, int);
extern void mos6502_dis_operand(mos6502 *, char *, int, int, int, vm_16bit);
extern void mos6502_dis_scan(mos6502 *, FILE *, int, int);

#endif
