#include "hid.h"

#include "../3ds.h"

DECL_PORT(hid) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x000a:
            cmd_params[0] = MAKE_IPCHEADER(1, 7);
            cmd_params[1] = 0;
            cmd_params[2] = 0x14000000;
            cmd_params[3] = handle_new(s);
            HANDLE_SET(cmd_params[3], &s->services.hid.sharedmem);
            s->services.hid.sharedmem.hdr.refcount++;
            for (int i = 0; i < HIDEVENT_MAX; i++) {
                cmd_params[4 + i] = handle_new(s);
                HANDLE_SET(cmd_params[4 + i], &s->services.hid.events[i]);
                s->services.hid.events[i].hdr.refcount++;
            }
            linfo("GetIPCHandles with sharedmem %x", cmd_params[3]);
            break;
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = -1;
            break;
    }
}
