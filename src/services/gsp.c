#include "gsp.h"

#include "../scheduler.h"
#include "../svc.h"

#define GSPMEM                                                                 \
    PTR(system->kernel.shmemblocks                                             \
            .d[HANDLE_VAL(system->services.gsp.memblock)]                      \
            .vaddr)

DECL_SRV(gsp_gpu) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0013:
            system->services.gsp.event = cmd_params[4];
            linfo("RegisterInterruptRelayQueue with event %d",
                  HANDLE_VAL(cmd_params[4]));
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

void handle_gsp_event(HLE3DS* system, u32 arg) {
    if (arg == GSPEVENT_VBLANK0) {
        add_event(&system->sched, EVENT_GSP, GSPEVENT_VBLANK0, CPU_CLK / 60);
        add_event(&system->sched, EVENT_GSP, GSPEVENT_VBLANK1, CPU_CLK / 60);
        system->frame_complete = true;
    }

    if (system->services.gsp.event == -1) return;

    struct {
        u8 cur;
        u8 count;
        u8 err;
        u8 flags;
        u8 errvblank0[4];
        u8 errvblank1[4];
        u8 queue[0x34];
    }* interrupts = GSPMEM;

    if (interrupts->count < 0x34) {
        u32 idx = interrupts->cur + interrupts->count;
        if (idx >= 0x34) idx -= 0x34;
        interrupts->queue[idx] = arg;
        interrupts->count++;
    }

    syncobj_signal(system, HANDLE_VAL(system->services.gsp.event));
}