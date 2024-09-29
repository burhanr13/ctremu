#include "kernel.h"

#include "3ds.h"

u32 handle_new(HLE3DS* s) {
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
    newNode->val = o;
    newNode->next = *l;
    *l = newNode;
}

void klist_remove(KListNode** l) {
    KListNode* cur = *l;
    *l = cur->next;
    free(cur);
}

void kobject_destroy(KObject* o) {
    switch (o->type) {
        default:
            free(o);
    }
}