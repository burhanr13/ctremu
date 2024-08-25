#ifndef X3DS_H
#define X3DS_H

#include "cpu.h"

#define FCRAMSIZE BIT(17)

typedef struct _3DS {
    CPU cpu;

    u8* memory;

    u32 free_memory;
} X3DS;

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

void* x3ds_mmap(X3DS* system, u32 addr, u32 size);

void x3ds_os_svc(X3DS* system, u32 num);

#endif
