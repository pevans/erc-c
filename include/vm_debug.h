#ifndef _VM_DEBUG_H_
#define _VM_DEBUG_H_

typedef struct {
    /*
     * Most commands that need an argument will simply use addr1, but a
     * few have more than one address--hence addr2.
     */
    int addr1;
    int addr2;

    /*
     * If we have a thing we want to work with, but want to leave what
     * that is up to the helper func, then you can write it into the
     * target.
     *
     * If a command uses target, followed by an address, that address
     * will be in addr1.
     */
    char target[256];
} vm_debug_args;

typedef void (*vm_debug_func)(vm_debug_args *);

typedef struct {
    /*
     * The name field is the full name of the command; each command also
     * has an abbreviated form (either is acceptable as input), which is
     * defined in the abbrev field.
     */
    char *name;
    char *abbrev;

    /*
     * The number of arguments we expect to see
     */
    int nargs;

    /*
     * The function that will do something with the command's input
     */
    vm_debug_func handler;

    /*
     * What do our arguments look like?
     */
    char *argdesc;

    /*
     * What do we do?
     */
    char *desc;
} vm_debug_cmd;

#define DEBUG_CMD(x) \
    void vm_debug_cmd_##x (vm_debug_args *args)

extern DEBUG_CMD(help);

#endif
