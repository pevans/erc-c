#ifndef _APPLE2_MEM_H_
#define _APPLE2_MEM_H_

#include "apple2.h"
#include "vm_segment.h"

/*
 * The MOS 6502 expects 64k of addressable space, but the Apple II
 * technically has 68k of memory in each bank. (Along with having both a
 * main and an auxiliary memory bank, with identical address schemes.)
 * This is handled through bank-switching; the additional memory (which
 * is accessible through $D000 - $DFFF) is actually stored in the
 * 64k-68k block at the end.
 */
#define APPLE2_MEMORY_SIZE 0x11000

/*
 * Given a slot number (1-7), this will return the memory page address
 * for the site of the ROM that is associated for the peripheral that
 * would be connected there.
 */
#define APPLE2_PERIPHERAL_SLOT(n) \
    (0xC000 + (n << 8))

/*
 * This is the size of any peripheral's ROM that we can hold in memory
 * (which is any page from $C100 - $C700).
 */
#define APPLE2_PERIPHERAL_SIZE 0x700

/*
 * Peripheral ROM can only occupy a single page in memory.
 */
#define APPLE2_PERIPHERAL_PAGE 0x100

/*
 * System ROM requires a full 16k; part of it is copied into several
 * pages beginning at $C800, and much more go into $D000 - $FFFF.
 */
#define APPLE2_SYSROM_SIZE 0x4000

/*
 * The size of our block of ROM is 12k
 */
#define APPLE2_ROM_SIZE 0x3000

/*
 * At the highest point (with the IIe extended 80 column text card), you
 * could have a whole other 64k of data in auxiliary memory. (Of which
 * only 1k was needed for 80 columns!)
 */
#define APPLE2_AUX_SIZE 0x10000

/*
 * This is the base address (or offset) for all bank-switched memory
 */
#define APPLE2_BANK_OFFSET 0xD000


extern SEGMENT_READER(apple2_mem_read_bank);
extern SEGMENT_WRITER(apple2_mem_write_bank);
extern int apple2_mem_init_peripheral_rom(apple2 *);
extern int apple2_mem_init_sys_rom(apple2 *);
extern void apple2_mem_map(apple2 *, vm_segment *);
extern void apple2_mem_map_bank_switch(vm_segment *);
extern SEGMENT_READER(apple2_mem_read_bank_switch);
extern SEGMENT_WRITER(apple2_mem_write_bank_switch);

#endif
