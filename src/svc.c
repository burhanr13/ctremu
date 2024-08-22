#include "svc.h"

#include <stdlib.h>

#include "svc_defs.h"

#define R(n) system->cpu.c.r[n]
#define PTR(addr) (void*) &system->memory[addr]

void n3ds_handle_svc(N3DS* system, u32 num) {
    num &= 0xff;
    if (!svc_table[num]) {
        lerror("unknown svc 0x%x (0x%x,0x%x,0x%x,0x%x) near %08x", num, R(0),
               R(1), R(2), R(3), system->cpu.c.pc);
        exit(1);
    }
    linfo("svc 0x%x (0x%x,0x%x,0x%x,0x%x)", num, R(0), R(1), R(2), R(3));
    svc_table[num](system);
}

DECL_SVC(ControlMemory) {
    u32 memop = R(0) & MEMOP_MASK;
    u32 memreg = R(0) & MEMOP_REGMASK;
    u32 linear = R(0) & MEMOP_LINEAR;
    u32 addr0 = R(1);
    u32 addr1 = R(2);
    u32 size = R(3);
    u32 perm = R(4);

    if (linear && !addr0) addr0 = lINEAR_HEAP_BASE;

    R(0) = 0;
    switch (memop) {
        case MEMOP_ALLOC:
            n3ds_mmap(system, addr0, size);
            R(1) = addr0;
            break;
        default:
            lwarn("unkown memory op for ControlMemory: %d", memop);
            R(0) = -1;
    }
}

DECL_SVC(CreateAddressArbiter) {
    R(0) = 0;
    R(1) = 1;
}

DECL_SVC(CloseHandle) {
    R(0) = 0;
}

DECL_SVC(GetResourceLimit) {
    R(0) = 0;
    R(1) = 1;
}

DECL_SVC(GetResourceLimitLimitValues) {
    s64* values = PTR(R(0));
    u32* names = PTR(R(2));
    s32 count = R(3);
    R(0) = 0;
    for (int i = 0; i < count; i++) {
        switch (names[i]) {
            case RES_MEMORY:
                values[i] = FCRAMSIZE;
                break;
            default:
                lwarn("unknown resource for GetResourceLimitLimitValues: %d",
                      names[i]);
                R(0) = -1;
        }
    }
}

DECL_SVC(GetResourceLimitCurrentValues) {
    s64* values = PTR(R(0));
    u32* names = PTR(R(2));
    s32 count = R(3);
    R(0) = 0;
    for (int i = 0; i < count; i++) {
        switch (names[i]) {
            case RES_MEMORY:
                values[i] = system->free_memory;
                break;
            default:
                lwarn("unknown resource for GetResourceLimitCurrentValues: %d",
                      names[i]);
                R(0) = -1;
        }
    }
}

DECL_SVC(Break) {
    lerror("Break svc");
    exit(1);
}

SVCFunc svc_table[SVC_MAX] = {
    [0x01] = svc_ControlMemory,
    [0x21] = svc_CreateAddressArbiter,
    [0x23] = svc_CloseHandle,
    [0x38] = svc_GetResourceLimit,
    [0x39] = svc_GetResourceLimitLimitValues,
    [0x3a] = svc_GetResourceLimitCurrentValues,
    [0x3c] = svc_Break,
};