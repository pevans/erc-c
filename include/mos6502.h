#ifndef _MOS6502_H_
#define _MOS6502_H_

#include <stdbool.h>

#include "vm_bits.h"
#include "vm_segment.h"

/*
 * The size of memory that the MOS 6502 supports is 64k (the limit of
 * values that a 16-bit address could possibly map to).
 */
#define MOS6502_MEMSIZE     65536

/*
 * This is a small macro to make it a bit simpler to set bytes ahead of
 * the PC register position; useful in testing.
 */
#define SET_PC_BYTE(cpu, off, byte) \
    mos6502_set(cpu, cpu->PC + off, byte)

/*
 * This macro is used to define new instruction handler functions.
 */
#define DEFINE_INST(inst) \
    void mos6502_handle_##inst (mos6502 *cpu, vm_8bit oper)

/*
 * In some address mode resolution, we must factor the carry bit into
 * the arithmetic we perform. In all those cases, if the carry bit is
 * set, we must only add 1 to the addition. The carry variable is,
 * therefore, the literal value we are adding, rather than a boolean
 * signifier.
 */
#define MOS_CARRY_BIT() \
    vm_8bit carry = 0; \
    if (cpu->P & MOS_CARRY) carry = 1

#define SET_RESULT(op) \
    int result = op

/*
 * A uniform way of declaring resolve functions for address modes, which
 * is useful in the event that we need to change the function signature.
 */
#define DECL_ADDR_MODE(x) \
    extern vm_8bit mos6502_resolve_##x (mos6502 *)

/*
 * Similarly, a uniform way of declaring instruction handler functions,
 * for the same reasons.
 */
#define DECL_INST(x) \
    extern void mos6502_handle_##x (mos6502 *, vm_8bit)

typedef struct {
    /*
     * There are two different segment pointers for reading and writing,
     * because it's possible for there to be two different banks in
     * which an action occurs. These memory segments must be injected at
     * creation time, and can be changed later.
     */
    vm_segment *rmem;
    vm_segment *wmem;

    /*
     * This contains the _effective_ address we've resolved in one
     * of our address modes. In absolute mode, this would be the literal
     * operand we read from memory; in indirect mode, this will be the
     * address we _find_ after dereferencing the operand we read from
     * memory. Another way of thinking of this is, this address is where
     * we found the value we care about.
     */
    vm_16bit eff_addr;

    /*
     * These are the last opcode and last effective address that was
     * used in the instruction previous to the one currently being
     * executed. Some things (notably soft switches) may need to
     * the last opcode.
     */
    vm_8bit last_opcode;
    vm_8bit last_operand;
    vm_16bit last_addr;

    /*
     * Our program counter register; this is what we'll use to determine
     * where we're "at" in memory while executing opcodes. We use a
     * 16-bit register because our memory is 64k large.
     */
    vm_16bit PC;

    /*
     * This is the accumulator register. It's used in most arithmetic
     * operations, and anything like that which you need to do will end
     * up storing the value here.
     */
    vm_8bit A;

    /*
     * The X and Y registers are our index registers. They're provided
     * to aid looping over tables, but they can also be used for other
     * purposes.
     */
    vm_8bit X, Y;

    /*
     * The P register is our status flag register. (I presume 'P' means
     * 'predicate'.) Each bit stands for some kind of status.
     */
    vm_8bit P;

    /*
     * The S register is our stack counter register. It indicates how
     * far into the stack we've gone.
     */
    vm_8bit S;
} mos6502;

/*
 * This is a small convenience so that we don't need to expose the
 * somewhat regrettable syntax for function pointers to any main source
 * file
 */
typedef vm_8bit (*mos6502_address_resolver)(mos6502 *);

/*
 * Another convenience; this type definition is for the functions we
 * write to handle instruction logic.
 */
typedef void (*mos6502_instruction_handler)(mos6502 *, vm_8bit);

extern bool mos6502_would_jump(int);
extern int mos6502_cycles(mos6502 *, vm_8bit);
extern int mos6502_instruction(vm_8bit);
extern mos6502 *mos6502_create(vm_segment *, vm_segment *);
extern mos6502_instruction_handler mos6502_get_instruction_handler(vm_8bit);
extern vm_16bit mos6502_get16(mos6502 *, size_t);
extern vm_8bit mos6502_get(mos6502 *, size_t);
extern vm_8bit mos6502_pop_stack(mos6502 *);
extern void mos6502_execute(mos6502 *);
extern void mos6502_free(mos6502 *);
extern void mos6502_last_executed(mos6502 *, vm_8bit *, vm_8bit *, vm_16bit *);
extern void mos6502_modify_status(mos6502 *, vm_8bit, int, int);
extern void mos6502_push_stack(mos6502 *, vm_8bit);
extern void mos6502_set(mos6502 *, size_t, vm_8bit);
extern void mos6502_set16(mos6502 *, size_t, vm_16bit);
extern void mos6502_set_memory(mos6502 *, vm_segment *, vm_segment *);
extern void mos6502_set_status(mos6502 *, vm_8bit);

/*
 * Below are some functions that are defined in mos6502.addr.c
 */
extern int mos6502_addr_mode(vm_8bit);
extern mos6502_address_resolver mos6502_get_address_resolver(int);

/*
 * All of our address modes
 */
DECL_ADDR_MODE(acc);
DECL_ADDR_MODE(abs);
DECL_ADDR_MODE(abx);
DECL_ADDR_MODE(aby);
DECL_ADDR_MODE(imm);
DECL_ADDR_MODE(ind);
DECL_ADDR_MODE(idx);
DECL_ADDR_MODE(idy);
DECL_ADDR_MODE(rel);
DECL_ADDR_MODE(zpg);
DECL_ADDR_MODE(zpx);
DECL_ADDR_MODE(zpy);

/*
 * And now, our instruction handlers; held generally in mos6502.*.c
 * (excepting mos6502.addr.c).
 */
DECL_INST(adc);
DECL_INST(and);
DECL_INST(asl);
DECL_INST(bad);
DECL_INST(bcc);
DECL_INST(bcs);
DECL_INST(beq);
DECL_INST(bit);
DECL_INST(bim);
DECL_INST(bmi);
DECL_INST(bne);
DECL_INST(bpl);
DECL_INST(bra);
DECL_INST(brk);
DECL_INST(bvc);
DECL_INST(bvs);
DECL_INST(clc);
DECL_INST(cld);
DECL_INST(cli);
DECL_INST(clv);
DECL_INST(cmp);
DECL_INST(cpx);
DECL_INST(cpy);
DECL_INST(dec);
DECL_INST(dex);
DECL_INST(dey);
DECL_INST(eor);
DECL_INST(inc);
DECL_INST(inx);
DECL_INST(iny);
DECL_INST(jmp);
DECL_INST(jsr);
DECL_INST(lda);
DECL_INST(ldx);
DECL_INST(ldy);
DECL_INST(lsr);
DECL_INST(nop);
DECL_INST(ora);
DECL_INST(pha);
DECL_INST(php);
DECL_INST(phx);
DECL_INST(phy);
DECL_INST(pla);
DECL_INST(plp);
DECL_INST(plx);
DECL_INST(ply);
DECL_INST(rol);
DECL_INST(ror);
DECL_INST(rti);
DECL_INST(rts);
DECL_INST(sbc);
DECL_INST(sec);
DECL_INST(sed);
DECL_INST(sei);
DECL_INST(sta);
DECL_INST(stx);
DECL_INST(sty);
DECL_INST(stz);
DECL_INST(tax);
DECL_INST(tay);
DECL_INST(tsx);
DECL_INST(txa);
DECL_INST(txs);
DECL_INST(tya);

#endif
