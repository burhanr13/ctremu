#include "hid.h"

#include "../3ds.h"

DECL_SRV(hid) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x000a:
            linfo("GetIPCHandles");
            cmd_params[0] = MAKE_IPCHEADER(1, 7);
            cmd_params[1] = 0;
            cmd_params[2] = 0x14000000;
            cmd_params[3] = s->services.hid.memblock;
            for (int i = 0; i < HIDEVENT_MAX; i++) {
                cmd_params[4 + i] = s->services.hid.events[i];
            }
            break;
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = -1;
            break;
    }
}
