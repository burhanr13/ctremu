#ifndef GPU_H
#define GPU_H

#include "types.h"

enum {
    GPUREG_VIEWPORT_WIDTH = 0x41,
    GPUREG_VIEWPORT_INVW,
    GPUREG_VIEWPORT_HEIGHT,
    GPUREG_VIEWPORT_INVH,

    GPUREG_VIEWPORT_XY = 0x68,
};

typedef struct {
    struct {
        int x, y;
        float w1, w2, h1, h2;
    } view;
} GPU;

typedef union {
    u32 w;
    struct {
        u32 id : 16;
        u32 mask : 4;
        u32 nparams : 8;
        u32 unused : 3;
        u32 incmode : 1;
    };
} GPUCommand;

void gpu_run_command_list(GPU* gpu, u32* addr, u32 size);

#endif
