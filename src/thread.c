#include "thread.h"

#include "3ds.h"

void load_context(X3DS* system) {
    for (int i = 0; i < 16; i++) {
        system->cpu.r[i] =
            system->kernel.threads.d[system->kernel.cur_tid].context.r[i];
        system->cpu.cpsr.w =
            system->kernel.threads.d[system->kernel.cur_tid].context.cpsr;
    }
}

void save_context(X3DS* system) {
    for (int i = 0; i < 16; i++) {
        system->kernel.threads.d[system->kernel.cur_tid].context.r[i] =
            system->cpu.r[i];
        system->kernel.threads.d[system->kernel.cur_tid].context.cpsr =
            system->cpu.cpsr.w;
    }
}

void thread_init(X3DS* system, u32 entrypoint) {
    system->kernel.cur_tid =
        thread_create(system, entrypoint, STACK_BASE, 0x30, 0);

    load_context(system);
    system->kernel.threads.d[system->kernel.cur_tid].state = THRD_RUNNING;
}

u32 thread_create(X3DS* system, u32 entrypoint, u32 stacktop, u32 priority,
                  u32 arg) {
    if (SVec_full(system->kernel.threads)) return -1;
    linfo("creating thread entry %08x, stack %08x, priority %x, arg %x", entrypoint,
          stacktop, priority, arg);
    return SVec_push(system->kernel.threads, ((Thread){.context.arg = arg,
                                                       .context.sp = stacktop,
                                                       .context.pc = entrypoint,
                                                       .context.cpsr = M_USER,
                                                       .priority = priority,
                                                       .state = THRD_READY}));
}

bool thread_reschedule(X3DS* system) {
    if (CUR_THREAD.state == THRD_RUNNING) CUR_THREAD.state = THRD_READY;
    u32 nexttid = -1;
    u32 maxprio = 0;
    Vec_foreach(t, system->kernel.threads) {
        if (t->state == THRD_READY && t->priority >= maxprio) {
            maxprio = t->priority;
            nexttid = t - system->kernel.threads.d;
        }
    }
    if (nexttid == -1) return false;
    linfo("switching thread from %d to %d", system->kernel.cur_tid, nexttid);
    save_context(system);
    system->kernel.cur_tid = nexttid;
    load_context(system);
    return true;
}
