#ifndef SVC_H
#define SVC_H

#include "3ds.h"

#define SVC_MAX BIT(8)

typedef void (*SVCFunc)(N3DS* system);
#define DECL_SVC(name) void svc_##name(N3DS* system)

extern SVCFunc svc_table[SVC_MAX];

void n3ds_handle_svc(N3DS* system, u32 num);

#endif
