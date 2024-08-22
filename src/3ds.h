#ifndef N3DS_H
#define N3DS_H

#include "cpu.h"

#define FCRAMSIZE BIT(17)

typedef struct _3DS {
    CPU cpu;

    u8* memory;

    u32 free_memory;
} N3DS;

#define HEAP_BASE BIT(27)

#define STACK_BASE BIT(28)
#define STACK_SIZE BIT(14)

#define lINEAR_HEAP_BASE (BIT(28) + BIT(26))

#define TLS_BASE 0x1ff82000

void n3ds_init(N3DS* system, char* romfile);
void n3ds_destroy(N3DS* system);

void n3ds_run_frame(N3DS* system);

void* n3ds_mmap(N3DS* system, u32 addr, u32 size);

void n3ds_os_svc(N3DS* system, u32 num);

#endif
