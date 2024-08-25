#ifndef SVC_H
#define SVC_H

#include "3ds.h"

#define SVC_MAX BIT(8)

enum {
    HANDLE_PORT = 0xfffe0000,
};

typedef void (*SVCFunc)(X3DS* system);
#define DECL_SVC(name) void svc_##name(X3DS* system)

extern SVCFunc svc_table[SVC_MAX];

void x3ds_handle_svc(X3DS* system, u32 num);

#endif
