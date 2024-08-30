#ifndef THREAD_H
#define THREAD_H

#include "types.h"

typedef struct _3DS HLE3DS;

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

enum {
    SYNCOBJ_EVENT,
    SYNCOBJ_MUTEX,
    SYNCOBJ_SEM,

    SYNCOBJ_MAX
};

typedef struct {
    u32 type;
    u32 value;
    Vector(u32) waiting_thrds;
} SyncObj;

typedef struct {
    u32 addr;
    u32 tid;
} AddressThread;

#define THRD_MAX_PRIO 0x40

#define CUR_THREAD (system->kernel.threads.d[system->kernel.cur_tid])
#define CUR_TLS (TLS_BASE + TLS_SIZE * system->kernel.cur_tid)

void thread_init(HLE3DS* system, u32 entrypoint);
u32 thread_create(HLE3DS* system, u32 entrypoint, u32 stacktop, u32 priority,
                  u32 arg);
bool thread_reschedule(HLE3DS* system);

u32 syncobj_create(HLE3DS* system, u32 type);
bool syncobj_wait(HLE3DS* system, u32 id);
void syncobj_signal(HLE3DS* system, u32 id);

#endif
