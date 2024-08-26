#ifndef KERNEL_H
#define KERNEL_H

#include "memory.h"
#include "thread.h"
#include "types.h"

#define MAX_RES 32

typedef struct _KernelData {
    VMBlock vmblocks;

    StaticVector(SHMemBlock, MAX_RES) shmemblocks;

    StaticVector(Thread, MAX_RES) threads;
    u32 cur_tid;
    bool pending_thrd_resched;

    u32 used_memory;
} KernelData;

#endif