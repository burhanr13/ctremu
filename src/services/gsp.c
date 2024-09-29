#include "gsp.h"

#include "../kernel.h"
#include "../pica/gpu.h"
#include "../scheduler.h"
#include "../svc.h"

#define GSPMEM(off) PTR(s->services.gsp.sharedmem.vaddr + off)

DECL_PORT(gsp_gpu) {
    u32* cmd_params = PTR(cmd_addr);
    switch (cmd.command) {
        case 0x0008:
            linfo("FlushDataCache");
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            break;
        case 0x000c:
            linfo("TriggerCmdReqQueue");
            gsp_handle_command(s);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            break;
        case 0x0013: {
            cmd_params[0] = MAKE_IPCHEADER(2, 2);

            u32 shmemhandle = srvobj_make_handle(s,&s->services.gsp.sharedmem.hdr);
            if (!shmemhandle) {
                cmd_params[1] = -1;
                return;
            }

            KEvent* event = HANDLE_GET_TYPED(cmd_params[3], KOT_EVENT);
            if (!event) {
                lerror("invalid event handle");
                cmd_params[1] = -1;
                return;
            }

            s->services.gsp.event = event;
            event->hdr.refcount++;

            linfo("RegisterInterruptRelayQueue with event handle=%x, "
                  "shmemhandle=%x",
                  cmd_params[4], shmemhandle);
            cmd_params[1] = 0;
            cmd_params[2] = 0;
            cmd_params[3] = 0;
            cmd_params[4] = shmemhandle;
            break;
        }
        case 0x0016:
            linfo("AcquireRight");
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = 0;
            break;
        default:
            lwarn("unknown command 0x%04x", cmd.command);
            cmd_params[0] = MAKE_IPCHEADER(1, 0);
            cmd_params[1] = -1;
            break;
    }
}

void gsp_handle_event(HLE3DS* s, u32 arg) {
    if (arg == GSPEVENT_VBLANK0) {
        add_event(&s->sched, gsp_handle_event, GSPEVENT_VBLANK0, CPU_CLK / 60);
        add_event(&s->sched, gsp_handle_event, GSPEVENT_VBLANK1, CPU_CLK / 60);
        s->frame_complete = true;
    }

    if (!s->services.gsp.sharedmem.mapped) return;

    struct {
        u8 cur;
        u8 count;
        u8 err;
        u8 flags;
        u8 errvblank0[4];
        u8 errvblank1[4];
        u8 queue[0x34];
    }* interrupts = GSPMEM(0);

    if (interrupts->count < 0x34) {
        u32 idx = interrupts->cur + interrupts->count;
        if (idx >= 0x34) idx -= 0x34;
        interrupts->queue[idx] = arg;
        interrupts->count++;
    }

    if (s->services.gsp.event) {
        linfo("signaling gsp event");
        event_signal(s, s->services.gsp.event);
    }
}

void gsp_handle_command(HLE3DS* s) {
    struct {
        u8 cur;
        u8 count;
        u8 flags;
        u8 flags2;
        u8 err;
        u8 pad[27];
        struct {
            u8 id;
            u8 unk;
            u8 unk2;
            u8 mode;
            u32 args[7];
        } d[15];
    }* cmds = GSPMEM(0x800);

    if (cmds->count == 0) return;

    switch (cmds->d[cmds->cur].id) {
        case 0x01: {
            u32 bufaddr = cmds->d[cmds->cur].args[0];
            u32 bufsize = cmds->d[cmds->cur].args[1];
            linfo("sending command list at %08x with size 0x%x", bufaddr,
                  bufsize);
            gpu_run_command_list(&s->gpu, PTR(bufaddr & ~7), bufsize / 4);
            gsp_handle_event(s, GSPEVENT_P3D);
            break;
        }
        case 0x02: {
            struct {
                u32 id;
                struct {
                    u32 st;
                    u32 val;
                    u32 end;
                } buf[2];
                u16 ctl[2];
            }* cmd = (void*) &cmds->d[cmds->cur];
            for (int i = 0; i < 2; i++) {
                if (cmd->buf[i].st) {
                    linfo("memory fill at fb %08x-%08x with %x", cmd->buf[i].st,
                          cmd->buf[i].end, cmd->buf[i].val);
                    if (i == 0) {
                        gpu_clear_fb(&s->gpu, cmd->buf[i].val);
                    }
                    gsp_handle_event(s, GSPEVENT_PSC0 + i);
                }
            }
            break;
        }
        case 0x03: {
            u32 addrin = cmds->d[cmds->cur].args[0];
            u32 addrout = cmds->d[cmds->cur].args[1];
            u32 dimin = cmds->d[cmds->cur].args[2];
            u32 dimout = cmds->d[cmds->cur].args[3];
            u32 flags = cmds->d[cmds->cur].args[4];
            linfo("display transfer from fb at %08x to %08x", addrin, addrout);
            gsp_handle_event(s, GSPEVENT_PPF);
            break;
        }
        case 0x05:
            linfo("flush cache regions");
            break;
        default:
            lwarn("unknown gsp queue command 0x%02x", cmds->d[cmds->cur].id);
    }

    cmds->cur++;
    if (cmds->cur >= 15) cmds->cur -= 15;
    cmds->count--;
}