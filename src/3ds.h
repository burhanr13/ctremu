#ifndef _3DS_H
#define _3DS_H

#include "arm/arm_core.h"
#include "memory.h"
#include "types.h"

#define FCRAMSIZE BIT(27)

typedef struct _3DS {
    ArmCore cpu;

    u8* virtmem;

    VMBlock vmblocks;

    u32 used_memory;
} X3DS;

#define R(n) system->cpu.r[n]
#define PTR(addr) (void*) &system->virtmem[addr]

#define PAGE_SIZE BIT(12)

#define HEAP_BASE BIT(27)

#define STACK_BASE BIT(28)
#define STACK_SIZE BIT(14)

#define lINEAR_HEAP_BASE (BIT(28) + BIT(26))

#define CONFIG_MEM 0x1ff80000
#define TLS_BASE 0x1ff82000

void x3ds_init(X3DS* system, char* romfile);
void x3ds_destroy(X3DS* system);

void x3ds_run_frame(X3DS* system);

#endif
