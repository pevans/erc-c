#ifndef _APPLE2_MEM_H_
#define _APPLE2_MEM_H_

#include "apple2.h"
#include "vm_segment.h"

#define APPLE2_DISK2_ROM_OFFSET 0xC600
#define APPLE2_DISK2_ROM_SIZE 0x100

/*
 * The size of our block of ROM is 12k
 */
#define APPLE2_ROM_SIZE 0x3000

/*
 * Whereas the second bank of RAM is a mere 4k
 */
#define APPLE2_RAM2_SIZE 0x1000

/*
 * This is the base address (or offset) for all bank-switched memory
 */
#define APPLE2_BANK_OFFSET 0xD000


extern vm_8bit apple2_mem_read_bank(vm_segment *, size_t, void *);
extern void apple2_mem_write_bank(vm_segment *, size_t, vm_8bit, void *);
extern void apple2_mem_map(apple2 *);
extern int apple2_mem_init_disk2_rom(apple2 *);
extern int apple2_mem_init_sys_rom(apple2 *);

#endif
