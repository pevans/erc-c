#ifndef _VM_BITS_H_
#define _VM_BITS_H_

#include <stdlib.h>

/*
 * We use the 8bit and 16bit types everywhere within the "vm"
 * abstraction; they are also used where applicable within the other
 * systems, like the mos6502 cpu, within the apple2 machine, etc.
 */
typedef uint8_t vm_8bit;
typedef uint16_t vm_16bit;

#endif
