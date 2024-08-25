#include "svc.h"

#include <stdlib.h>
#include <string.h>

#include "srv.h"
#include "svc_defs.h"

void x3ds_handle_svc(X3DS* system, u32 num) {
    num &= 0xff;
    if (!svc_table[num]) {
        lerror("unknown svc 0x%x (0x%x,0x%x,0x%x,0x%x) at %08x (lr=%08x)", num,
               R(0), R(1), R(2), R(3), system->cpu.pc, system->cpu.lr);
        return;
    }
    linfo("handling svc 0x%x: %s(0x%x,0x%x,0x%x,0x%x,0x%x)", num,
          svc_names[num], R(0), R(1), R(2), R(3), R(4));
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
            x3ds_vmalloc(system, addr0, size, perm, MEMST_PRIVATE);
            R(1) = addr0;
            break;
        default:
            lwarn("unknown memory op %d", memop);
            R(0) = -1;
    }
}

DECL_SVC(QueryMemory) {
    u32 addr = R(2);
    VMBlock* b = x3ds_vmquery(system, addr);
    R(0) = 0;
    R(1) = b->startpg << 12;
    R(2) = (b->endpg - b->startpg) << 12;
    R(3) = b->perm;
    R(4) = b->state;
    R(5) = 0;
}

DECL_SVC(CreateEvent) {
    R(0) = 0;
    R(1) = 1;
}

DECL_SVC(CreateAddressArbiter) {
    R(0) = 0;
    R(1) = 1;
}

DECL_SVC(CloseHandle) {
    R(0) = 0;
}

DECL_SVC(ConnectToPort) {
    char* port = PTR(R(1));
    if (!strcmp(port, "srv:")) {
        linfo("connected to port '%s'", port);
        R(0) = 0;
        R(1) = MAKE_HANDLE(HANDLE_SESSION, SRV_SRV);
    } else {
        R(0) = -1;
        lwarn("unknown port '%s'", port);
    }
}

DECL_SVC(SendSyncRequest) {
    if (HANDLE_TYPE(R(0)) != HANDLE_SESSION || HANDLE_VAL(R(0)) >= SRV_MAX) {
        lerror("invalid session handle");
        R(0) = -1;
        return;
    }
    u32 cmd_addr = TLS_BASE + IPC_CMD_OFF;
    IPCHeader cmd = {*(u32*) PTR(cmd_addr)};
    handle_service_request(system, HANDLE_VAL(R(0)), cmd,
                           TLS_BASE + IPC_CMD_OFF);
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
                lwarn("unknown resource %d", names[i]);
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
                values[i] = system->used_memory;
                break;
            default:
                lwarn("unknown resource %d", names[i]);
                R(0) = -1;
        }
    }
}

DECL_SVC(Break) {
    lerror("Break svc");
    exit(1);
}

DECL_SVC(OutputDebugString) {
    printf("%*s", R(1), PTR(R(0)));
}

SVCFunc svc_table[SVC_MAX] = {
    [0x01] = svc_ControlMemory,
    [0x02] = svc_QueryMemory,
    [0x21] = svc_CreateAddressArbiter,
    [0x23] = svc_CloseHandle,
    [0x2d] = svc_ConnectToPort,
    [0x32] = svc_SendSyncRequest,
    [0x38] = svc_GetResourceLimit,
    [0x39] = svc_GetResourceLimitLimitValues,
    [0x3a] = svc_GetResourceLimitCurrentValues,
    [0x3c] = svc_Break,
    [0x3d] = svc_OutputDebugString,
};

char* svc_names[SVC_MAX] = {
    [0x01] = "ControlMemory",
    [0x02] = "QueryMemory",
    [0x17] = "CreateEvent",
    [0x21] = "CreateAddressArbiter",
    [0x23] = "CloseHandle",
    [0x2d] = "ConnectToPort",
    [0x32] = "SendSyncRequest",
    [0x38] = "GetResourceLimit",
    [0x39] = "GetResourceLimitLimitValues",
    [0x3a] = "GetResourceLimitCurrentValues",
    [0x3c] = "Break",
    [0x3d] = "OutputDebugString",
};