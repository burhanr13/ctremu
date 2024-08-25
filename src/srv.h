#ifndef SRV_H
#define SRV_H

#include "3ds.h"
#include "types.h"

enum {
    SRV_SRV,
    SRV_GSP_GPU,

    SRV_MAX
};

typedef union {
    u32 w;
    struct {
        u32 paramsize_translate : 6;
        u32 paramsize_normal : 6;
        u32 unused : 4;
        u32 command : 16;
    };
} IPCHeader;

void handle_service_request(X3DS* system, u32 srv, IPCHeader cmd, u32 cmd_addr);

typedef void (*SRVFunc)(X3DS* system, IPCHeader cmd, u32 cmd_addr);
#define DECL_SRV(name)                                                         \
    void handle_srv_##name(X3DS* system, IPCHeader cmd, u32 cmd_addr)

#endif
