#ifndef GSP_H
#define GSP_H

#include "../srv.h"
#include "../svc_types.h"

enum {
    GSPEVENT_PSC0,
    GSPEVENT_PSC1,
    GSPEVENT_VBLANK0,
    GSPEVENT_VBLANK1,
    GSPEVENT_PPF,
    GSPEVENT_P3D,
    GSPEVENT_DMA,
};

typedef struct {
    Handle event;
    Handle memblock;
} GSPData;

DECL_SRV(gsp_gpu);

void gsp_handle_event(HLE3DS* s, u32 arg);
void gsp_handle_command(HLE3DS* s);

#endif
