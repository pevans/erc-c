#ifndef _VM_BITFONT_H_
#define _VM_BITFONT_H_

#include <SDL.h>

#include "log.h"
#include "vm_screen.h"

typedef struct {
    SDL_Texture *texture;

    int width;
    int height;

    char cmask;
} vm_bitfont;

extern int vm_bitfont_render(vm_bitfont *, vm_screen *, vm_area *, char);
extern vm_bitfont *vm_bitfont_create(vm_screen *, const char *, int, int, char);
extern void vm_bitfont_free(vm_bitfont *);
extern void vm_bitfont_offset(vm_bitfont *, char, vm_area *);

#endif
