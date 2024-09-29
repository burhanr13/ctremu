#include "srv.h"

#include <string.h>

#include "svc.h"

#include "services/apt.h"
#include "services/fs.h"
#include "services/gsp.h"
#include "services/hid.h"

void init_services(HLE3DS* s) {
    s->services.gsp.event = NULL;
    s->services.gsp.sharedmem.hdr.type = KOT_SHAREDMEM;
    s->services.gsp.sharedmem.hdr.refcount = 1;

    s->services.hid.sharedmem.hdr.type = KOT_SHAREDMEM;
    s->services.hid.sharedmem.hdr.refcount = 1;
    for (int i = 0; i < HIDEVENT_MAX; i++) {
        s->services.hid.events[i].hdr.type = KOT_EVENT;
        s->services.hid.events[i].hdr.refcount = 1;
    }
}

KSession* session_create(PortRequestHandler f) {
    KSession* session = calloc(1, sizeof *session);
    session->hdr.type = KOT_SESSION;
    session->handler = f;
    return session;
}

DECL_PORT(srv) {
#define IS(_name) (!strcmp(name, _name))
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0005: {
            char name[9];
            memcpy(name, &cmd_params[1], cmd_params[3]);
            name[cmd_params[3]] = '\0';
            cmd_params[0] = MAKE_IPCHEADER(3, 0);
            cmd_params[1] = 0;

            PortRequestHandler handler;
            if (IS("APT:U") || IS("APT:A") || IS("APT:S")) {
                handler = port_handle_apt;
            } else if (IS("fs:USER")) {
                handler = port_handle_fs;
            } else if (IS("gsp::Gpu")) {
                handler = port_handle_gsp_gpu;
            } else if (IS("hid:USER") || IS("hid:SPVR")) {
                handler = port_handle_hid;
            } else {
                lerror("unknown service '%s'", name);
                cmd_params[1] = -1;
            }
            if (cmd_params[1] == 0) {
                u32 handle = handle_new(s);
                KSession* session = session_create(handler);
                HANDLE_SET(handle, session);
                session->hdr.refcount = 1;
                cmd_params[3] = handle;
                linfo("connected to service '%s' with handle %x", name, handle);
            }
            break;
        }
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            break;
    }
#undef IS
}
