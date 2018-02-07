#ifndef _VM_REFLECT_H_
#define _VM_REFLECT_H_

#include <stdio.h>

/*
 * Some forward decls for vm_reflect
 */
struct vm_reflect;
typedef struct vm_reflect vm_reflect;

/*
 * A reflect function, mostly helpful in the struct definition itself as
 * well as a function decls and defns. A reflect function would accept a
 * reflect object and use that to figure out what it's talking about.
 */
typedef void (*vm_reflect_fn)(vm_reflect *);

struct vm_reflect {
    /*
     * These are the machine and CPU objects that the reflect system
     * would want to work with.
     */
    void *machine;
    void *cpu;

    /*
     * Where information that we print will go to.
     */
    FILE *stream;

    /*
     * Print out information about the machine and CPU, respectively.
     */
    vm_reflect_fn cpu_info;
    vm_reflect_fn machine_info;

    /*
     * This function will pause or resume operation of the virtual machine.
     */
    vm_reflect_fn pause;

    /*
     * Turn on, or off, disassembly of the instructions being executed.
     */
    vm_reflect_fn disasm;

    /*
     * Eventually we will have the ability to load and save state to a
     * file mechanism (probably not the one defined as `stream` above).
     */
#if 0
    vm_reflect_fn save_state;
    vm_reflect_fn load_state;
#endif
};

/*
 * Allow us to either declare or define a new reflect function that can
 * be hooked into a reflect struct
 */
#define REFLECT(x) \
    void x(vm_reflect *ref)

extern int vm_reflect_cpu_info(vm_reflect *);
extern int vm_reflect_disasm(vm_reflect *);
extern int vm_reflect_machine_info(vm_reflect *);
extern int vm_reflect_pause(vm_reflect *);
extern vm_reflect *vm_reflect_create();
extern void vm_reflect_free(vm_reflect *);

#endif
