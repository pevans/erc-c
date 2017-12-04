/*
 * mos6502.bits.c
 */

#include "mos6502.h"
#include "mos6502.enums.h"

DEFINE_INST(and)
{
    cpu->A &= oper;
}

DEFINE_INST(asl)
{
    cpu->P &= ~CARRY;
    if (oper & 0x80) {
        cpu->P |= CARRY;
    }

    oper <<= 1;

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}

DEFINE_INST(bit)
{
    cpu->P &= ~NEGATIVE;
    if (oper & NEGATIVE) {
        cpu->P |= NEGATIVE;
    }

    cpu->P &= ~OVERFLOW;
    if (oper & OVERFLOW) {
        cpu->P |= OVERFLOW;
    }

    if (oper & cpu->A) {
        cpu->P &= ~ZERO;
    } else {
        cpu->P |= ZERO;
    }
}

DEFINE_INST(eor)
{
    cpu->A ^= oper;
}

DEFINE_INST(lsr)
{
    cpu->P &= ~CARRY;
    if (oper & 0x01) {
        cpu->P |= CARRY;
    }

    oper >>= 1;

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}

DEFINE_INST(ora)
{
    cpu->A |= oper;
}

DEFINE_INST(rol)
{
    CARRY_BIT();

    if (oper & 0x80) {
        carry = 1;
    }

    oper <<= 1;

    if (carry) {
        oper |= 0x01;
    }

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}

DEFINE_INST(ror)
{
    CARRY_BIT();

    if (oper & 0x01) {
        carry = 1;
    }

    oper >>= 1;

    if (carry) {
        oper |= 0x80;
    }

    if (cpu->last_addr) {
        vm_segment_set(cpu->memory, cpu->last_addr, oper);
    } else {
        cpu->A = oper;
    }
}
