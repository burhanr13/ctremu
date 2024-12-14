#include "svc.h"

#include <stdlib.h>
#include <string.h>

#include "srv.h"
#include "svc_types.h"
#include "thread.h"

#define MAKE_HANDLE(handle)                                                    \
    u32 handle = handle_new(s);                                                \
    if (!handle) {                                                             \
        R(0) = -1;                                                             \
        return;                                                                \
    }

void e3ds_handle_svc(E3DS* s, u32 num) {
    num &= 0xff;
    if (!svc_table[num]) {
        lerror("unknown svc 0x%x (0x%x,0x%x,0x%x,0x%x) at %08x (lr=%08x)", num,
               R(0), R(1), R(2), R(3), s->cpu.pc, s->cpu.lr);
        return;
    }
    linfo("svc 0x%x: %s(0x%x,0x%x,0x%x,0x%x,0x%x) at %08x (lr=%08x)", num,
          svc_names[num], R(0), R(1), R(2), R(3), R(4), R(15), R(14));
    svc_table[num](s);
}

DECL_SVC(ControlMemory) {
    u32 memop = R(0) & MEMOP_MASK;
    u32 memreg = R(0) & MEMOP_REGMASK;
    bool linear = R(0) & MEMOP_LINEAR;
    u32 addr0 = R(1);
    u32 addr1 = R(2);
    u32 size = R(3);
    u32 perm = R(4);

    if (linear && !addr0) addr0 = LINEAR_HEAP_BASE;

    R(0) = 0;
    switch (memop) {
        case MEMOP_ALLOC:
            e3ds_vmmap(s, addr0, size, perm, MEMST_PRIVATE, linear);
            R(1) = addr0;
            break;
        default:
            lwarn("unknown memory op %d", memop);
            R(0) = -1;
    }
}

DECL_SVC(QueryMemory) {
    u32 addr = R(2);
    VMBlock* b = e3ds_vmquery(s, addr);
    R(0) = 0;
    R(1) = b->startpg << 12;
    R(2) = (b->endpg - b->startpg) << 12;
    R(3) = b->perm;
    R(4) = b->state;
    R(5) = 0;
}

DECL_SVC(CreateThread) {
    s32 priority = R(0);
    u32 entrypoint = R(1);
    u32 arg = R(2);
    u32 stacktop = R(3);

    MAKE_HANDLE(handle);

    if (priority < 0 || priority >= THRD_MAX_PRIO) {
        R(0) = -1;
        return;
    }
    stacktop &= ~7;

    u32 newtid = thread_create(s, entrypoint, stacktop, priority, arg);
    if (newtid == -1) {
        R(0) = -1;
        return;
    }

    HANDLE_SET(handle, s->process.threads[newtid]);
    s->process.threads[newtid]->hdr.refcount = 1;
    linfo("created thread with handle %x", handle);

    R(0) = 0;
    R(1) = handle;

    if (priority < CUR_THREAD->priority) thread_reschedule(s);
}

DECL_SVC(ExitThread) {
    linfo("thread %d exiting", CUR_THREAD->id);

    CUR_THREAD->state = THRD_DEAD;
    KListNode** cur = &CUR_THREAD->waiting_thrds;
    while (*cur) {
        thread_wakeup(s, (KThread*) (*cur)->key, &CUR_THREAD->hdr);
        klist_remove(cur);
    }

    s->process.threads[CUR_THREAD->id] = NULL;

    thread_reschedule(s);
}

DECL_SVC(SleepThread) {
    s64 timeout = R(0) | (u64) R(1) << 32;

    R(0) = 0;
    thread_sleep(s, CUR_THREAD, timeout);
    thread_reschedule(s);
}

DECL_SVC(GetThreadPriority) {
    KThread* t = HANDLE_GET_TYPED(R(1), KOT_THREAD);
    if (!t) {
        lerror("not a thread");
        R(0) = -1;
        return;
    }

    R(0) = 0;
    R(1) = t->priority;
}

DECL_SVC(SetThreadPriority) {
    KThread* t = HANDLE_GET_TYPED(R(0), KOT_THREAD);
    if (!t) {
        lerror("not a thread");
        R(0) = -1;
        return;
    }

    R(0) = 0;
    t->priority = R(1);
    thread_reschedule(s);
}

DECL_SVC(CreateMutex) {
    MAKE_HANDLE(handle);

    KMutex* mtx = mutex_create();
    if (R(1)) mtx->locker_thrd = CUR_THREAD;
    mtx->hdr.refcount = 1;
    HANDLE_SET(handle, mtx);

    linfo("created mutex with handle %x", handle);

    R(0) = 0;
    R(1) = handle;
}

DECL_SVC(ReleaseMutex) {
    KMutex* m = HANDLE_GET_TYPED(R(0), KOT_MUTEX);
    if (!m) {
        lerror("not a mutex");
        R(0) = -1;
        return;
    }

    linfo("releasing mutex %x", R(0));

    R(0) = 0;
    mutex_release(s, m);
}

DECL_SVC(CreateEvent) {
    MAKE_HANDLE(handle);

    KEvent* event = event_create(R(1) == RESET_STICKY);
    event->hdr.refcount = 1;
    HANDLE_SET(handle, event);

    linfo("created event with handle %x, sticky=%d", handle,
          R(1) == RESET_STICKY);

    R(0) = 0;
    R(1) = handle;
}

DECL_SVC(SignalEvent) {
    KEvent* e = HANDLE_GET_TYPED(R(0), KOT_EVENT);
    if (!e) {
        lerror("not an event");
        R(0) = -1;
        return;
    }

    linfo("signaling event %x", R(0));

    R(0) = 0;
    event_signal(s, e);
}

DECL_SVC(ClearEvent) {
    KEvent* e = HANDLE_GET_TYPED(R(0), KOT_EVENT);
    if (!e) {
        lerror("not an event");
        R(0) = -1;
        return;
    }
    R(0) = 0;
    e->signal = false;
}

DECL_SVC(CreateMemoryBlock) {
    MAKE_HANDLE(handle);

    u32 addr = R(1);
    u32 size = R(2);
    u32 perm = R(3);

    KSharedMem* shm = calloc(1, sizeof *shm);
    shm->vaddr = addr;
    shm->size = size;
    shm->mapped = true;
    shm->hdr.refcount = 1;
    HANDLE_SET(handle, shm);

    linfo("created memory block with handle %x at addr %x", handle, addr);

    e3ds_vmmap(s, addr, size, perm, MEMST_SHARED, false);

    R(0) = 0;
    R(1) = handle;
}

DECL_SVC(MapMemoryBlock) {
    u32 memblock = R(0);
    u32 addr = R(1);
    u32 perm = R(2);

    KSharedMem* shmem = HANDLE_GET_TYPED(memblock, KOT_SHAREDMEM);

    if (!shmem) {
        R(0) = -1;
        lerror("invalid memory block handle");
        return;
    }

    if (addr && shmem->vaddr) lwarn("remapping a shared mem block");

    if (addr) {
        shmem->vaddr = addr;
    } else if (shmem->vaddr) {
        addr = shmem->vaddr;
    } else {
        lwarn("trying to map at NULL");
        R(0) = -1;
        return;
    }
    shmem->mapped = true;

    linfo("mapping shared mem block %x at %08x", memblock, addr);

    e3ds_vmmap(s, addr, shmem->size ? shmem->size : PAGE_SIZE, perm,
               MEMST_SHARED, false);

    if (shmem->defaultdata) {
        memcpy(PTR(addr), shmem->defaultdata, shmem->defaultdatalen);
    }

    R(0) = 0;
}

DECL_SVC(CreateAddressArbiter) {
    MAKE_HANDLE(h);

    KArbiter* arbiter = calloc(1, sizeof *arbiter);
    arbiter->hdr.type = KOT_ARBITER;
    arbiter->hdr.refcount = 1;
    linfo("handle=%x", h);

    HANDLE_SET(h, arbiter);

    R(0) = 0;
    R(1) = h;
}

DECL_SVC(ArbitrateAddress) {
    u32 h = R(0);
    u32 addr = R(1);
    u32 type = R(2);
    s32 value = R(3);
    s64 time = R(4) | (u64) R(5) << 32;

    KArbiter* arbiter = HANDLE_GET_TYPED(h, KOT_ARBITER);
    if (!arbiter) {
        lerror("bad typed handle");
        R(0) = -1;
        return;
    }

    R(0) = 0;

    switch (type) {
        case ARBITRATE_SIGNAL:
            linfo("signaling address %08x", addr);
            if (value < 0) {
                KListNode** cur = &arbiter->waiting_thrds;
                while (*cur) {
                    KThread* t = (KThread*) (*cur)->key;
                    if (t->waiting_addr == addr) {
                        thread_wakeup(s, t, &arbiter->hdr);
                        klist_remove(cur);
                    } else {
                        cur = &(*cur)->next;
                    }
                }
            } else {
                for (int i = 0; i < value; i++) {
                    u32 maxprio = THRD_MAX_PRIO;
                    KListNode** toRemove = NULL;
                    KListNode** cur = &arbiter->waiting_thrds;
                    while (*cur) {
                        KThread* t = (KThread*) (*cur)->key;
                        if (t->waiting_addr == addr) {
                            if (t->priority < maxprio) {
                                maxprio = t->priority;
                                toRemove = cur;
                            }
                        }
                        cur = &(*cur)->next;
                    }
                    if (!toRemove) break;
                    KThread* t = (KThread*) (*toRemove)->key;
                    thread_wakeup(s, t, &arbiter->hdr);
                    klist_remove(toRemove);
                }
            }
            thread_reschedule(s);
            break;
        case ARBITRATE_WAIT:
        case ARBITRATE_DEC_WAIT:
            if (*(s32*) PTR(addr) < value) {
                klist_insert(&arbiter->waiting_thrds, &CUR_THREAD->hdr);
                klist_insert(&CUR_THREAD->waiting_objs, &arbiter->hdr);
                CUR_THREAD->waiting_addr = addr;
                linfo("waiting on address %08x", addr);
                thread_sleep(s, CUR_THREAD, -1);
                thread_reschedule(s);
            }
            if (type == ARBITRATE_DEC_WAIT) {
                *(s32*) PTR(addr) -= 1;
            }
            break;
        default:
            R(0) = -1;
            lwarn("unknown arbitration type");
    }
}

DECL_SVC(CloseHandle) {
    u32 handle = R(0);
    KObject* obj = HANDLE_GET(handle);
    if (!obj) {
        lerror("invalid handle");
        R(0) = 0;
        return;
    }
    R(0) = 0;
    if (!--obj->refcount) {
        linfo("destroying object of handle %x", handle);
        HANDLE_SET(handle, NULL);
        kobject_destroy(s, obj);
    }
}

DECL_SVC(WaitSynchronization1) {
    u32 handle = R(0);

    s64 timeout = R(2) | (u64) R(3) << 32;

    KObject* obj = HANDLE_GET(handle);
    if (!obj) {
        lerror("invalid handle");
        R(0) = -1;
        return;
    }
    R(0) = 0;

    if (sync_wait(s, obj)) {
        linfo("waiting on handle %x", handle);
        klist_insert(&CUR_THREAD->waiting_objs, obj);
        thread_sleep(s, CUR_THREAD, timeout);
        thread_reschedule(s);
    } else {
        linfo("did not need to wait for handle %x", handle);
    }
}

DECL_SVC(WaitSynchronizationN) {
    u32* handles = PTR(R(1));
    int count = R(2);
    bool waitAll = R(3);
    s64 timeout = R(0) | (u64) R(4) << 32;

    bool wokeup = false;
    int wokeupi = 0;

    for (int i = 0; i < count; i++) {
        KObject* obj = HANDLE_GET(handles[i]);
        if (!obj) {
            lerror("invalid handle %x", handles[i]);
            continue;
        }
        if (sync_wait(s, obj)) {
            linfo("waiting on handle %x", handles[i]);
            klist_insert(&CUR_THREAD->waiting_objs, obj);
            CUR_THREAD->waiting_objs->val = i;
        } else {
            linfo("handle %x already waited", handles[i]);
            wokeup = true;
            wokeupi = i;
            break;
        }
    }

    R(0) = 0;

    if ((!waitAll && wokeup) || !CUR_THREAD->waiting_objs) {
        linfo("did not need to wait");
        KListNode** cur = &CUR_THREAD->waiting_objs;
        while (*cur) {
            sync_cancel(CUR_THREAD, (*cur)->key);
            klist_remove(cur);
        }
        R(1) = wokeupi;
    } else {
        linfo("waiting on %d handles", count);
        CUR_THREAD->wait_all = waitAll;
        thread_sleep(s, CUR_THREAD, timeout);
        thread_reschedule(s);
    }
}

DECL_SVC(DuplicateHandle) {
    KObject* o = HANDLE_GET(R(1));
    if (!o) {
        lerror("invalid handle");
        R(0) = -1;
        return;
    }
    MAKE_HANDLE(dup);
    o->refcount++;
    HANDLE_SET(dup, o);

    R(0) = 0;
}

DECL_SVC(GetSystemTick) {
    R(0) = s->sched.now;
    R(1) = s->sched.now >> 32;
    s->sched.now += 200; // make time advance so the next read happens later
}

DECL_SVC(GetProcessInfo) {
    u32 type = R(2);

    R(0) = 0;
    switch (type) {
        case 0x14: // linear memory address conversion
            R(1) = FCRAM_PBASE - LINEAR_HEAP_BASE;
            R(2) = 0;
            break;
        default:
            R(0) = -1;
            lerror("unknown process info");
            break;
    }
}

DECL_SVC(ConnectToPort) {
    MAKE_HANDLE(h);
    char* port = PTR(R(1));
    R(0) = 0;
    if (!strcmp(port, "srv:")) {
        linfo("connected to port '%s' with handle %x", port, h);
        KSession* session = session_create(port_handle_srv);
        session->hdr.refcount = 1;
        HANDLE_SET(h, session);
        R(1) = h;
    } else if (!strcmp(port, "err:f")) {
        linfo("connected to port '%s' with handle %x", port, h);
        KSession* session = session_create(port_handle_errf);
        session->hdr.refcount = 1;
        HANDLE_SET(h, session);
        R(1) = h;
    } else {
        R(0) = -1;
        lerror("unknown port '%s'", port);
    }
}

DECL_SVC(SendSyncRequest) {
    KSession* session = HANDLE_GET_TYPED(R(0), KOT_SESSION);
    if (!session) {
        lerror("invalid session handle %x", R(0));
        R(0) = 0;
        return;
    }
    R(0) = 0;
    u32 cmd_addr = CUR_TLS + IPC_CMD_OFF;
    IPCHeader cmd = *(IPCHeader*) PTR(cmd_addr);
    session->handler(s, cmd, CUR_TLS + IPC_CMD_OFF, session->arg);
}

DECL_SVC(GetProcessId) {
    R(0) = 0;
    R(1) = 0;
}

DECL_SVC(GetThreadId) {
    KThread* t = HANDLE_GET_TYPED(R(1), KOT_THREAD);
    if (!t) {
        lerror("not a thread");
        R(0) = -1;
    }

    R(0) = 0;
    R(1) = t->id;
}

DECL_SVC(GetResourceLimit) {
    MAKE_HANDLE(h);
    R(0) = 0;
    KObject* dummy = malloc(sizeof *dummy);
    dummy->type = KOT_RESLIMIT;
    dummy->refcount = 1;
    HANDLE_SET(h, dummy);
    R(1) = h;
}

DECL_SVC(GetResourceLimitLimitValues) {
    s64* values = PTR(R(0));
    u32* names = PTR(R(2));
    s32 count = R(3);
    R(0) = 0;
    for (int i = 0; i < count; i++) {
        switch (names[i]) {
            case RES_MEMORY:
                values[i] = FCRAMSIZE;
                linfo("memory: %08x", values[i]);
                break;
            default:
                lwarn("unknown resource %d", names[i]);
                R(0) = -1;
        }
    }
}

DECL_SVC(GetResourceLimitCurrentValues) {
    s64* values = PTR(R(0));
    u32* names = PTR(R(2));
    s32 count = R(3);
    R(0) = 0;
    for (int i = 0; i < count; i++) {
        switch (names[i]) {
            case RES_MEMORY:
                values[i] = s->process.used_memory;
                linfo("memory: %08x", values[i]);
                break;
            default:
                lwarn("unknown resource %d", names[i]);
                R(0) = -1;
        }
    }
}

DECL_SVC(Break) {
    lerror("at %08x (lr=%08x)", s->cpu.pc, s->cpu.lr);
    exit(1);
}

DECL_SVC(OutputDebugString) {
    printf("%*s\n", R(1), (char*) PTR(R(0)));
}

SVCFunc svc_table[SVC_MAX] = {
#define SVC(num, name) [num] = svc_##name,
#include "svcs.inc"
#undef SVC
};

char* svc_names[SVC_MAX] = {
#define SVC(num, name) [num] = #name,
#include "svcs.inc"
#undef SVC
};