#include "dsp.h"

#include "../3ds.h"

DECL_PORT(dsp) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0011:
            linfo("LoadComponent");
            cmd_params[0] = MAKE_IPCHEADER(2, 2);
            cmd_params[1] = 0;
            cmd_params[2] = true;
            cmd_params[3] = cmd_params[4];
            cmd_params[4] = cmd_params[5];
            break;
        case 0x0015:
            linfo("RegisterInterruptEvents");
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            break;
        case 0x0016:
            linfo("GetSemaphoreEventHandle");
            cmd_params[0] = MAKE_IPCHEADER(1, 2);
            cmd_params[1] = 0;
            cmd_params[2] = 0;
            cmd_params[3] = 0x12345678;
            break;
        case 0x001f:
            linfo("GetHeadphoneStatus");
            cmd_params[0] = MAKE_IPCHEADER(2, 0);
            cmd_params[1] = 0;
            cmd_params[2] = false;
            break;
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            break;
    }
}
