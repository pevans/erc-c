#ifndef _VM_DI_H_
#define _VM_DI_H_

enum vm_di_entry {
    VM_CPU,
    VM_MACHINE,
    VM_REFLECT,

    // This value is the size of the DI container we will construct. As
    // you can see, it's quite a bit higher than what would be implied
    // by the number of enum values currently defined--and it is so we
    // don't have to update this value every time we add a new value.
    // But be wary that you don't add so many entries as to go beyond
    // VM_DI_SIZE's defined value!
    VM_DI_SIZE = 100,
};

extern void *vm_di_get(int);
extern int vm_di_set(int, void *);

#endif
