#include "gsp.h"

#include "../svc.h"

DECL_SRV(gsp_gpu) {    
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0013:
            linfo("RegisterInterruptRelayQueue");
            cmd_params[0] =
                (IPCHeader){.paramsize_normal = 2, .paramsize_translate = 2}.w;
            cmd_params[1] = 0;
            cmd_params[2] = 0;
            cmd_params[3] = 0;
            cmd_params[4] = system->services.gsp.memblock;
            break;
        case 0x0016:
            linfo("AcquireRight");
            cmd_params[0] = (IPCHeader){.paramsize_normal = 1}.w;
            cmd_params[1] = 0;
            break;
        default:
            lwarn("unknown command 0x%04x", cmd.command);
    }
}