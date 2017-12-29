#ifndef _MOS6502_DIS_H_
#define _MOS6502_DIS_H_

#include "vm_bits.h"
#include "vm_segment.h"

extern bool mos6502_dis_is_jump_label(int);
extern int mos6502_dis_expected_bytes(int);
extern int mos6502_dis_opcode(FILE *, vm_segment *, int);
extern void mos6502_dis_instruction(FILE *, int);
extern void mos6502_dis_jump_label(vm_16bit, int, int);
extern void mos6502_dis_jump_unlabel(int);
extern void mos6502_dis_label(FILE *, int);
extern void mos6502_dis_operand(FILE *, int, int, vm_16bit);
extern void mos6502_dis_scan(FILE *, vm_segment *, int, int);

#endif
