#include "srv.h"

#include <string.h>

#include "svc.h"

#include "services/apt.h"
#include "services/fs.h"
#include "services/gsp.h"
#include "services/hid.h"

extern SRVFunc srv_funcs[SRV_MAX];

void init_services(HLE3DS* s) {
    s->services.gsp.event = -1;
    s->services.gsp.memblock = MAKE_HANDLE(
        HANDLE_MEMBLOCK, SVec_push(s->kernel.shmemblocks, (SHMemBlock){0}));
    s->services.hid.memblock = MAKE_HANDLE(
        HANDLE_MEMBLOCK, SVec_push(s->kernel.shmemblocks, (SHMemBlock){0}));
}

void services_handle_request(HLE3DS* s, u32 srv, IPCHeader cmd, u32 cmd_addr) {
    srv_funcs[srv](s, cmd, cmd_addr);
}

DECL_SRV(srv) {
#define IS(_name) (!strcmp(name, _name))
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0005: {
            char name[9];
            memcpy(name, &cmd_params[1], cmd_params[3]);
            name[cmd_params[3]] = '\0';
            cmd_params[0] = MAKE_IPCHEADER(3, 0);
            cmd_params[1] = 0;

            u32 srvid;
            if (IS("APT:U") || IS("APT:A") || IS("APT:S")) {
                srvid = SRV_APT;
            } else if (IS("fs:USER")) {
                srvid = SRV_FS;
            } else if (IS("gsp::Gpu")) {
                srvid = SRV_GSP_GPU;
            } else if (IS("hid:USER") || IS("hid:SPVR")) {
                srvid = SRV_HID;
            } else {
                lerror("unknown service '%s'", name);
                cmd_params[1] = -1;
            }
            if (cmd_params[1] == 0) {
                cmd_params[3] = MAKE_HANDLE(HANDLE_SESSION, srvid);
                linfo("connected to service '%s'", name);
            }
            break;
        }
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            break;
    }
#undef IS
}

SRVFunc srv_funcs[SRV_MAX] = {
    [SRV_SRV] = srv_handle_srv, [SRV_APT] = srv_handle_apt,
    [SRV_FS] = srv_handle_fs,   [SRV_GSP_GPU] = srv_handle_gsp_gpu,
    [SRV_HID] = srv_handle_hid,
};