#ifndef _VM_DEBUG_H_
#define _VM_DEBUG_H_

#include <stdbool.h>

struct vm_debug_args;
typedef struct vm_debug_args vm_debug_args;

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
     * The function that will do something with the command's input
     */
    vm_debug_func handler;

    /*
     * The number of arguments we expect to see
     */
    int nargs;

    /*
     * What do our arguments look like?
     */
    char *argdesc;

    /*
     * What do we do?
     */
    char *desc;
} vm_debug_cmd;

struct vm_debug_args {
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
    char *target;

    /*
     * The command our arguments are attached to; from here we can call
     * the handler with ourselves. (Very meta.)
     */
    vm_debug_cmd *cmd;
};

#define DEBUG_CMD(x) \
    void vm_debug_cmd_##x (vm_debug_args *args)

extern bool vm_debug_broke(int);
extern char *vm_debug_prompt();
extern vm_debug_cmd *vm_debug_find_cmd(const char *);
extern void vm_debug_break(int);
extern void vm_debug_execute(const char *);
extern void vm_debug_quit();
extern void vm_debug_unbreak(int);
extern void vm_debug_unbreak_all();

extern DEBUG_CMD(break);
extern DEBUG_CMD(help);
extern DEBUG_CMD(jump);
extern DEBUG_CMD(printaddr);
extern DEBUG_CMD(printstate);
extern DEBUG_CMD(quit);
extern DEBUG_CMD(resume);
extern DEBUG_CMD(unbreak);
extern DEBUG_CMD(writeaddr);
extern DEBUG_CMD(writestate);

#endif
