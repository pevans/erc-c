/*
 * vm_reflect.c
 *
 * Here we have support for reflection, or perhaps meta-manipulation, of
 * the virtual machine. You can create hooks to stop the machine, or
 * disassemble opcodes from it, or handle state.
 */

#include <stdlib.h>

#include "log.h"
#include "vm_di.h"
#include "vm_reflect.h"

/*
 * Create a new vm_reflect struct with the given machine, cpu and
 * stream.
 */
vm_reflect *
vm_reflect_create()
{
    vm_reflect *ref;

    ref = malloc(sizeof(vm_reflect));
    if (ref == NULL) {
        log_critical("Could not allocate memory for vm_reflect");
        return NULL;
    }

    ref->machine = vm_di_get(VM_MACHINE);
    ref->cpu = vm_di_get(VM_CPU);
    ref->stream = vm_di_get(VM_OUTPUT);

    ref->cpu_info = NULL;
    ref->machine_info = NULL;
    ref->pause = NULL;
    ref->disasm = NULL;

    return ref;
}

/*
 * Free a vm_reflect struct that we created earlier
 */
void
vm_reflect_free(vm_reflect *ref)
{
    // Not much to this--just going to free the main memory chunk
    free(ref);
}

/*
 * All of the reflect functions do essentially the same thing--at least,
 * right now they do.
 */
#define REFLECT_HANDLER(x) \
    int vm_reflect_##x(vm_reflect *ref) { \
        if (ref == NULL) ref = (vm_reflect *)vm_di_get(VM_REFLECT); \
        if (ref->x == NULL) return ERR_INVALID; ref->x(ref); return OK; }

REFLECT_HANDLER(cpu_info);      // ignore docblock
REFLECT_HANDLER(machine_info);  // ignore docblock
REFLECT_HANDLER(pause);         // ignore docblock
REFLECT_HANDLER(disasm);        // ignore docblock
