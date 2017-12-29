#ifndef _MOS6502_DIS_H_
#define _MOS6502_DIS_H_

#include "vm_bits.h"
#include "vm_segment.h"

extern int mos6502_dis_expected_bytes(vm_8bit);
extern int mos6502_dis_scan(FILE *, vm_segment *, int);
extern void mos6502_dis_instruction(FILE *, int);
extern void mos6502_dis_operand(FILE *, int, vm_16bit);

#endif
