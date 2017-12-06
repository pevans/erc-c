/*
 * apple2.c
 */

#include "apple2.h"

apple2 *
apple2_create()
{
    apple2 *mach;

    mach = malloc(sizeof(apple2));
    if (mach == NULL) {
        return NULL;
    }

    mach->cpu = mos6502_create();
    
    return mach;
}
