#include "3ds.h"

#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "loader.h"
#include "svc_types.h"

void hle3ds_init(HLE3DS* s, char* romfile) {
    memset(s, 0, sizeof *s);

    s->sched.master = s;

    cpu_init(s);

    hle3ds_memory_init(s);

    u32 entrypoint = 0;

    char* ext = strrchr(romfile, '.');
    if (!ext) {
        eprintf("unsupported file format\n");
        exit(1);
    }
    if (!strcmp(ext, ".elf")) {
        entrypoint = load_elf(s, romfile);
    } else if (!strcmp(ext, ".3ds")) {
        entrypoint = load_ncsd(s, romfile);
    } else {
        eprintf("unsupported file format\n");
        exit(1);
    }

    hle3ds_vmmap(s, STACK_BASE - STACK_SIZE, STACK_SIZE, PERM_RW, MEMST_PRIVATE,
                 false);

    hle3ds_vmmap(s, DSPMEM, DSPBUFBIT, PERM_RW, MEMST_STATIC, false);
    hle3ds_vmmap(s, DSPMEM | DSPBUFBIT, DSPBUFBIT, PERM_RW, MEMST_STATIC,
                 false);

    hle3ds_vmmap(s, CONFIG_MEM, PAGE_SIZE, PERM_R, MEMST_STATIC, false);
    ((u32*) PTR(CONFIG_MEM))[0x10] = FCRAMSIZE;

    hle3ds_vmmap(s, SHARED_PAGE, PAGE_SIZE, PERM_R, MEMST_STATIC, false);

    hle3ds_vmmap(s, TLS_BASE, TLS_SIZE * THREAD_MAX, PERM_RW, MEMST_PRIVATE,
                 false);

    init_services(s);

    thread_init(s, entrypoint);

    s->process.hdr.type = KOT_PROCESS;
    s->process.hdr.refcount = 1;
    s->process.handles[1] = &s->process.hdr;

    add_event(&s->sched, gsp_handle_event, GSPEVENT_VBLANK0, CPU_CLK / FPS);
}

void hle3ds_destroy(HLE3DS* s) {
    cpu_free(s);

    if (s->gamecard.fp) fclose(s->gamecard.fp);

    hle3ds_memory_destroy(s);
}

void hle3ds_run_frame(HLE3DS* s) {
    while (!s->frame_complete) {
        if (!s->cpu.wfe) {
            cpu_run(s, FIFO_peek(s->sched.event_queue).time - s->sched.now);
        }
        run_next_event(&s->sched);
        run_to_present(&s->sched);
        while (s->cpu.wfe && !s->frame_complete) {
            run_next_event(&s->sched);
        }
    }
    s->frame_complete = false;
}