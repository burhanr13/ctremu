#ifndef THREAD_H
#define THREAD_H

#include "types.h"

typedef struct _3DS X3DS;

enum {
    THRD_RUNNING,
    THRD_READY,
    THRD_SLEEP,
};

typedef struct {
    struct {
        union {
            u32 r[16];
            struct {
                u32 arg;
                u32 _r[12];
                u32 sp;
                u32 lr;
                u32 pc;
            };
        };
        u32 cpsr;
    } context;

    s32 priority;
    u32 state;
} Thread;

#define THRD_MAX_PRIO 0x40

#define CUR_THREAD (system->kernel.threads.d[system->kernel.cur_tid])
#define CUR_TLS (TLS_BASE + TLS_SIZE * system->kernel.cur_tid)

void thread_init(X3DS* system, u32 entrypoint);
u32 thread_create(X3DS* system, u32 entrypoint, u32 stacktop, u32 priority,
                  u32 arg);
bool thread_reschedule(X3DS* system);

#endif
