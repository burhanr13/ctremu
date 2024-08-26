#ifndef _3DS_H
#define _3DS_H

#include "arm/arm_core.h"
#include "kernel.h"
#include "memory.h"
#include "types.h"
#include "srv.h"
#include "service_data.h"

#define CPU_CLK BIT(28)
#define FPS 60

#define FCRAMSIZE BIT(27)

typedef struct _3DS {
    ArmCore cpu;

    u64 now;

    u8* virtmem;

    KernelData kernel;

    ServiceData services;
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
#define TLS_SIZE 0x200
#define IPC_CMD_OFF 0x80

void x3ds_init(X3DS* system, char* romfile);
void x3ds_destroy(X3DS* system);

void x3ds_run_frame(X3DS* system);

#endif
