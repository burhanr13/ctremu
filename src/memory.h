#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

typedef struct _3DS X3DS;

typedef struct _VMBlock {
    u32 startpg;
    u32 endpg;
    u32 perm;
    u32 state;
    struct _VMBlock* next;
    struct _VMBlock* prev;
} VMBlock;

typedef struct {
    u32 vaddr;
} SHMemBlock;

void x3ds_memory_init(X3DS* system);
void x3ds_memory_destroy(X3DS* system);

void x3ds_vmalloc(X3DS* system, u32 base, u32 size, u32 perm, u32 state);
VMBlock* x3ds_vmquery(X3DS* system, u32 addr);

#endif
