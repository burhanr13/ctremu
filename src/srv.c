#include "srv.h"

#include <stdio.h>
#include <string.h>

#include "svc.h"

#include "services/gsp.h"

extern SRVFunc srv_funcs[SRV_MAX];

void init_services(X3DS* system) {
    system->services.gsp.memblock =
        MAKE_HANDLE(HANDLE_MEMBLOCK,
                    SVec_push(system->kernel.shmemblocks, (SHMemBlock){0}));
}

void handle_service_request(X3DS* system, u32 srv, IPCHeader cmd,
                            u32 cmd_addr) {
    srv_funcs[srv](system, cmd, cmd_addr);
}

DECL_SRV(srv) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0005: {
            char name[9];
            memcpy(name, &cmd_params[1], cmd_params[3]);
            name[cmd_params[3]] = '\0';
            cmd_params[0] = (IPCHeader){.paramsize_normal = 3}.w;
            cmd_params[1] = 0;

            if (!strcmp(name, "gsp::Gpu")) {
                cmd_params[3] = MAKE_HANDLE(HANDLE_SESSION, SRV_GSP_GPU);
            } else {
                lwarn("unknown service '%s'", name);
                cmd_params[1] = -1;
            }
            if (cmd_params[1] == 0) {
                linfo("connected to service '%s'", name);
            }
            break;
        }
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            break;
    }
}

SRVFunc srv_funcs[SRV_MAX] = {
    [SRV_SRV] = handle_srv_srv,
    [SRV_GSP_GPU] = handle_srv_gsp_gpu,
};