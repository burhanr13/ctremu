#ifndef SRV_H
#define SRV_H

#include "types.h"

typedef struct _3DS HLE3DS;

enum {
    SRV_SRV,

    SRV_APT,
    SRV_FS,
    SRV_GSP_GPU,
    SRV_HID,

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
#define MAKE_IPCHEADER(normal, translate)                                      \
    ((IPCHeader) {.paramsize_normal = normal,                                  \
                  .paramsize_translate = translate}                            \
         .w)

void init_services(HLE3DS* s);

void services_handle_request(HLE3DS* s, u32 srv, IPCHeader cmd, u32 cmd_addr);

typedef void (*SRVFunc)(HLE3DS* s, IPCHeader cmd, u32 cmd_addr);
#define DECL_SRV(name)                                                         \
    void srv_handle_##name(HLE3DS* s, IPCHeader cmd, u32 cmd_addr)

#endif
