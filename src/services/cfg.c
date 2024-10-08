#include "cfg.h"

#include "../3ds.h"

DECL_PORT(cfg) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0001: {
            int blkid = cmd_params[2];
            void* ptr = PTR(cmd_params[4]);
            linfo("GetConfigInfoBlk2 with blkid %x", blkid);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            switch (blkid) {
                case 0x50005: {
                    float* block = ptr;
                    block[0] = 62.0f;
                    block[1] = 289.0f;
                    block[2] = 76.80f;
                    block[3] = 46.08f;
                    block[4] = 10.0f;
                    block[5] = 5.0f;
                    block[6] = 55.58f;
                    block[7] = 21.57f;
                    break;
                }
                case 0x70001:
                    *(u8*) ptr = 0;
                    break;
                case 0xa0002:
                    *(u8*) ptr = 1;
                    break;
                case 0xb0000:
                    ((u8*) ptr)[2] = 1;
                    ((u8*) ptr)[3] = 49;
                    break;
                case 0xc0000:
                    ((u32*) ptr)[0] = 0;
                    ((u32*) ptr)[1] = 0;
                    ((u32*) ptr)[2] = 0;
                    ((u32*) ptr)[3] = 0;
                    ((u32*) ptr)[4] = 0;
                    break;
                case 0xd0000:
                    ((u16*) ptr)[0] = 1;
                    ((u16*) ptr)[1] = 1;
                    break;
                case 0x130000:
                    *(u32*) ptr = 0;
                    break;
                default:
                    lwarn("unknown blkid %x", blkid);
                    break;
            }
            break;
        }
        case 0x0002:
            linfo("GetRegion");
            cmd_params[0] = MAKE_IPCHEADER(2, 0);
            cmd_params[1] = 0;
            cmd_params[2] = 1;
            break;
        case 0x0003:
            linfo("GenHashConsoleUnique");
            cmd_params[0] = MAKE_IPCHEADER(3, 0);
            cmd_params[1] = 0;
            cmd_params[2] = 0x69696969;
            cmd_params[3] = 0x69696969;
            break;
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            break;
    }
}
