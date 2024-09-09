#ifndef SHADER_H
#define SHADER_H

#include "gpu.h"

typedef union {
    u32 w;
    struct {
        u32 desc : 7;
        u32 ops : 19;
        u32 opcode : 6;
    };
    struct {
        u32 desc : 7;
        u32 src2 : 5;
        u32 src1 : 7;
        u32 idx1 : 2;
        u32 dest : 5;
        u32 opcode : 6;
    } fmt1;
    struct {
        u32 desc : 7;
        u32 src2 : 7;
        u32 src1 : 5;
        u32 idx2 : 2;
        u32 dest : 5;
        u32 opcode : 6;
    } fmt1i;
} PICAInstr;

typedef union {
    u32 w;
    struct {
        u32 destmask : 4;
        u32 src1neg : 1;
        u32 src1swizzle : 8;
        u32 src2neg : 1;
        u32 src2swizzle : 8;
        u32 src3neg : 1;
        u32 src3swizzle : 8;
    };
} OpDesc;

void exec_vshader(GPU* gpu);

#endif
