#ifndef DSP_H
#define DSP_H

#include "../memory.h"
#include "../srv.h"
#include "../svc_types.h"
#include "../thread.h"

typedef struct {
    KEvent* event;
    KEvent semEvent;
} DSPData;

DECL_PORT(dsp);

#endif
