#ifndef GSP_H
#define GSP_H

#include "../srv.h"
#include "../svc_types.h"

typedef struct {
    Handle event;
    Handle memblock;
} GSPData;

DECL_SRV(gsp_gpu);

#endif
