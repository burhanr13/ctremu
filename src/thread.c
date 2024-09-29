#include "thread.h"

#include "3ds.h"

void load_context(HLE3DS* s) {
    for (int i = 0; i < 16; i++) {
        s->cpu.r[i] = CUR_THREAD->context.r[i];
        s->cpu.d[i] = CUR_THREAD->context.d[i];
    }
    s->cpu.cpsr.w = CUR_THREAD->context.cpsr;
    s->cpu.fpscr.w = CUR_THREAD->context.fpscr;
}

void save_context(HLE3DS* s) {
    for (int i = 0; i < 16; i++) {
        CUR_THREAD->context.r[i] = s->cpu.r[i];
        CUR_THREAD->context.d[i] = s->cpu.d[i];
    }
    CUR_THREAD->context.cpsr = s->cpu.cpsr.w;
    CUR_THREAD->context.fpscr = s->cpu.fpscr.w;
}

void thread_init(HLE3DS* s, u32 entrypoint) {
    u32 tid = thread_create(s, entrypoint, STACK_BASE, 0x30, 0);

    s->process.handles[0] = &s->process.threads[tid]->hdr;

    load_context(s);
    CUR_THREAD->state = THRD_RUNNING;
}

u32 thread_create(HLE3DS* s, u32 entrypoint, u32 stacktop, u32 priority,
                  u32 arg) {
    u32 tid = -1;
    for (int i = 0; i < THREAD_MAX; i++) {
        if (!s->process.threads[i]) {
            tid = i;
            break;
        }
    }
    if (tid == -1) {
        lerror("not enough threads");
        return tid;
    }
    KThread* thrd = calloc(1, sizeof *thrd);
    thrd->hdr.type = KOT_THREAD;
    thrd->context.arg = arg;
    thrd->context.sp = stacktop;
    thrd->context.pc = entrypoint;
    thrd->context.cpsr = M_USER;
    thrd->priority = priority;
    thrd->state = THRD_READY;
    thrd->id = tid;
    s->process.threads[tid] = thrd;

    linfo("creating thread %d (entry %08x, stack %08x, priority %x, arg %x)",
          tid, entrypoint, stacktop, priority, arg);
    return tid;
}

bool thread_reschedule(HLE3DS* s) {
    if (CUR_THREAD->state == THRD_RUNNING) CUR_THREAD->state = THRD_READY;
    u32 nexttid = -1;
    u32 maxprio = THRD_MAX_PRIO;
    for (u32 i = 0; i < THREAD_MAX; i++) {
        KThread* t = s->process.threads[i];
        if (!t) continue;
        if (t->state == THRD_READY && t->priority < maxprio) {
            maxprio = t->priority;
            nexttid = i;
        }
    }
    if (nexttid == -1) {
        s->cpu.wfe = true;
        linfo("all threads sleeping");
        return false;
    }
    linfo("switching thread from %d to %d", CUR_THREAD->id, nexttid);
    save_context(s);
    s->process.handles[0] = &s->process.threads[nexttid]->hdr;
    load_context(s);
    return true;
}

void thread_wakeup(KThread* t, KObject* reason) {
    t->state = THRD_READY;
}

KEvent* event_create() {
    KEvent* ev = calloc(1, sizeof *ev);
    ev->hdr.type = KOT_EVENT;
    return ev;
}

void event_signal(HLE3DS* s, KEvent* ev) {
    KListNode** cur = &ev->waiting_thrds;
    while (*cur) {
        thread_wakeup((KThread*) (*cur)->val, &ev->hdr);
        klist_remove(cur);
    }
    thread_reschedule(s);
}

void event_wait(HLE3DS* s, KEvent* ev) {
    klist_insert(&ev->waiting_thrds, &CUR_THREAD->hdr);
    CUR_THREAD->state = THRD_SLEEP;
    thread_reschedule(s);
}