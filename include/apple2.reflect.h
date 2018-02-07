#ifndef _APPLE2_REFLECT_H_
#define _APPLE2_REFLECT_H_

#include "vm_reflect.h"

extern void apple2_reflect_init();

extern REFLECT(apple2_reflect_cpu_info);
extern REFLECT(apple2_reflect_disasm);
extern REFLECT(apple2_reflect_machine_info);
extern REFLECT(apple2_reflect_pause);

#endif
