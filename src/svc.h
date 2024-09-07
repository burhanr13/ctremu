#ifndef SVC_H
#define SVC_H

#include "3ds.h"

#define SVC_MAX BIT(8)

enum {
    HANDLE_SESSION = 0x0001,
    HANDLE_SYNCOBJ = 0x0002,
    HANDLE_MEMBLOCK = 0x0003,
    HANDLE_THREAD = 0x0004,
};

#define MAKE_HANDLE(type, val) (type << 16 | val)
#define HANDLE_TYPE(h) ((u32) h >> 16)
#define HANDLE_VAL(h) (h & 0xffff)

typedef void (*SVCFunc)(HLE3DS* s);
#define DECL_SVC(name) void svc_##name(HLE3DS* s)

extern SVCFunc svc_table[SVC_MAX];
extern char* svc_names[SVC_MAX];

void hle3ds_handle_svc(HLE3DS* s, u32 num);

#endif
