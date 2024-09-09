#include "shader.h"

void readsrc(GPU* gpu, fvec src, u32 n, u8 swizzle, bool neg) {
    fvec* rn = (n < 0x10)   ? &gpu->in[n]
               : (n < 0x20) ? &gpu->reg[n - 0x10]
                            : &gpu->cst[n - 0x20];
    for (int i = 0; i < 4; i++) {
        src[i] = (*rn)[(swizzle >> 2 * (3 - i)) & 3];
        if (neg) src[i] = -src[i];
    }
}

void writedest(GPU* gpu, fvec dest, u32 n, u8 mask) {
    fvec* rn = (n < 0x10) ? &gpu->out[n] : &gpu->reg[n - 0x10];
    for (int i = 0; i < 4; i++) {
        if (mask & BIT(3 - i)) (*rn)[i] = dest[i];
    }
}

#define SRC(v, i, _fmt)                                                        \
    readsrc(gpu, v, instr.fmt##_fmt.src##i, desc.src##i##swizzle,              \
            desc.src##i##neg)
#define SRC1(v, fmt) SRC(v, 1, fmt)
#define SRC2(v, fmt) SRC(v, 2, fmt)
#define SRC3(v, fmt) SRC(v, 3, fmt)
#define DEST(v, _fmt) writedest(gpu, v, instr.fmt##_fmt.dest, desc.destmask)

bool exec_instr(GPU* gpu, PICAInstr instr) {
    OpDesc desc = {gpu->opdescs[instr.desc]};
    switch (instr.opcode) {
        case 0x02: {
            fvec a, b;
            SRC1(a, 1);
            SRC2(b, 1);
            fvec res;
            res[0] = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
            res[1] = res[2] = res[3] = res[0];
            DEST(res, 1);
            break;
        }
        case 0x13: {
            fvec a;
            SRC1(a, 1);
            DEST(a, 1);
            break;
        }
        case 0x22:
            return false;
        default:
            lerror("unknown PICA instruction %08x", instr.w);
    }
    return true;
}

void exec_vshader(GPU* gpu) {
    u32 pc = gpu->io.VSH.entrypoint;
    while (exec_instr(gpu, (PICAInstr) {gpu->progdata[pc++]}));
}