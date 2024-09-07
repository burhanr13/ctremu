#include "gpu.h"

static inline float fromf32(u32 i) {
    return ((union {
               u32 i;
               float f;
           }) {i})
        .f;
}

static inline float fromf24(u32 i) {
    u32 sgn = (i >> 23) & i;
    u32 exp = (i >> 16) & 0x7f;
    u32 mantissa = i & 0xffff;
    if (exp == 0 && mantissa == 0) {
        exp = 0;
    } else if (exp == 0x7f) {
        exp = 0xff;
    } else {
        exp += 0x40;
    }
    mantissa <<= 7;
    i = sgn << 31 | exp << 23 | mantissa;
    return fromf32(i);
}

static inline float fromf31(u32 i) {
    i >>= 1;
    u32 sgn = (i >> 30) & i;
    u32 exp = (i >> 23) & 0x7f;
    u32 mantissa = i & 0x7fffff;
    if (exp == 0 && mantissa == 0) {
        exp = 0;
    } else if (exp == 0x7f) {
        exp = 0xff;
    } else {
        exp += 0x40;
    }
    i = sgn << 31 | exp << 23 | mantissa;
    return fromf32(i);
}

void gpu_run_command(GPU* gpu, u16 id, u32 param, u8 mask) {
    switch (id) {
        case GPUREG_VIEWPORT_WIDTH:
            gpu->view.w1 = 2 * fromf24(param);
            linfo("viewport w: %f", gpu->view.w1);
            break;
        case GPUREG_VIEWPORT_INVW:
            gpu->view.w2 = 2 / fromf31(param);
            linfo("viewport w2: %f", gpu->view.w2);
            break;
        case GPUREG_VIEWPORT_HEIGHT:
            gpu->view.h1 = 2 * fromf24(param);
            linfo("viewport h: %f", gpu->view.h1);
            break;
        case GPUREG_VIEWPORT_INVH:
            gpu->view.h2 = 2 / fromf31(param);
            linfo("viewport h2: %f", gpu->view.h2);
            break;
        case GPUREG_VIEWPORT_XY:
            gpu->view.x = param & 0xffff;
            gpu->view.x <<= 6;
            gpu->view.x >>= 6;
            gpu->view.y = param >> 16;
            gpu->view.y <<= 6;
            gpu->view.y >>= 6;
            linfo("viewport xy: %d,%d", gpu->view.x, gpu->view.y);
            break;
        default:
            lwarn("unknown GPU command id %x param %x mask %x", id, param,
                  mask);
    }
}

void gpu_run_command_list(GPU* gpu, u32* addr, u32 size) {
    u32* cur = addr;
    u32* end = addr + size;
    while (cur < end) {
        GPUCommand c = {cur[1]};
        gpu_run_command(gpu, c.id, cur[0], c.mask);
        cur += 2;
        if (c.incmode) c.id++;
        for (int i = 0; i < c.nparams; i++) {
            gpu_run_command(gpu, c.id, *cur++, c.mask);
            if (c.incmode) c.id++;
        }
        if (c.nparams & 1) cur++;
    }
}