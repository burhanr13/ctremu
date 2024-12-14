#include "kernel.h"

#include "3ds.h"

u32 handle_new(E3DS* s) {
    for (int i = 0; i < HANDLE_MAX; i++) {
        if (!s->process.handles[i]) {
            return HANDLE_BASE + i;
        }
    }
    lerror("no free handles");
    return 0;
}

void klist_insert(KListNode** l, KObject* o) {
    KListNode* newNode = malloc(sizeof *newNode);
    newNode->key = o;
    newNode->next = *l;
    *l = newNode;
}

void klist_remove(KListNode** l) {
    KListNode* cur = *l;
    *l = cur->next;
    free(cur);
}

u32 klist_remove_key(KListNode** l, KObject* o) {
    while (*l) {
        if ((*l)->key == o) {
            u32 v = (*l)->val;
            klist_remove(l);
            return v;
        }
        l = &(*l)->next;
    }
    return 0;
}

void kobject_destroy(E3DS* s, KObject* o) {
    switch (o->type) {
        case KOT_THREAD: {
            KThread* t = (KThread*) o;
            t->state = THRD_DEAD;
            KListNode** cur = &t->waiting_thrds;
            while (*cur) {
                thread_wakeup(s, (KThread*) (*cur)->key, &t->hdr);
                klist_remove(cur);
            }

            s->process.threads[t->id] = NULL;

            thread_reschedule(s);
            break;
        }
        case KOT_EVENT: {
            KEvent* e = (KEvent*) o;
            if (!e->waiting_thrds) {
                free(e);
            }
            break;
        }
        case KOT_SESSION:
        case KOT_RESLIMIT:
            free(o);
            break;
        default:
            lwarn("unimpl free of a kobject type %d", o->type);
    }
}