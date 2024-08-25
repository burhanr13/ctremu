#include "srv.h"

#include <stdio.h>
#include <string.h>

#include "svc.h"

#define PTR(addr) (void*) &system->memory[addr]

SRVFunc srv_funcs[SRV_MAX];

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
                cmd_params[3] = HANDLE_PORT + SRV_GSP_GPU;
            } else {
                lwarn("unimpl service '%s'", name);
                cmd_params[1] = -1;
            }
            if (cmd_params[1] == 0) {
                linfo("connected to service '%s'", name);
            }
            break;
        }
        default:
            lwarn("unimpl command 0x%04x", cmd.command);
            break;
    }
}

DECL_SRV(gsp_gpu) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        default:
            lwarn("unimpl command 0x%04x", cmd.command);
    }
}

SRVFunc srv_funcs[SRV_MAX] = {
    [SRV_SRV] = handle_srv_srv,
    [SRV_GSP_GPU] = handle_srv_gsp_gpu,
};