#ifndef SRV_H
#define SRV_H

#include "types.h"

typedef struct _3DS HLE3DS;

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

void init_services(HLE3DS* system);

void handle_service_request(HLE3DS* system, u32 srv, IPCHeader cmd,
                            u32 cmd_addr);

typedef void (*SRVFunc)(HLE3DS* system, IPCHeader cmd, u32 cmd_addr);
#define DECL_SRV(name)                                                         \
    void handle_srv_##name(HLE3DS* system, IPCHeader cmd, u32 cmd_addr)

#endif
