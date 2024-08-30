#include "3ds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "emulator_state.h"
#include "loader.h"
#include "svc_types.h"

void hle3ds_init(HLE3DS* system, char* romfile) {
    memset(system, 0, sizeof *system);

    system->sched.master = system;

    cpu_init(system);

    hle3ds_memory_init(system);

    u32 entrypoint = 0;

    char* ext = strrchr(romfile, '.');
    if (!ext) {
        eprintf("unsupported file format\n");
        exit(1);
    }
    if (!strcmp(ext, ".elf")) {
        entrypoint = load_elf(system, romfile);
    } else {
        eprintf("unsupported file format\n");
        exit(1);
    }

    hle3ds_vmalloc(system, STACK_BASE - STACK_SIZE, STACK_SIZE, PERM_RW,
                   MEMST_PRIVATE);

    hle3ds_vmalloc(system, CONFIG_MEM, PAGE_SIZE, PERM_R, MEMST_STATIC);
    hle3ds_vmalloc(system, TLS_BASE, TLS_SIZE * MAX_RES, PERM_RW,
                   MEMST_PRIVATE);

    init_services(system);

    thread_init(system, entrypoint);

    add_event(&system->sched, EVENT_GSP, GSPEVENT_VBLANK0, CPU_CLK / FPS);
}

void hle3ds_destroy(HLE3DS* system) {
    cpu_free(system);

    hle3ds_memory_destroy(system);
}

void hle3ds_run_frame(HLE3DS* system) {
    while (!system->frame_complete) {
        cpu_run(system,
                FIFO_peek(system->sched.event_queue).time - system->sched.now);
        run_next_event(&system->sched);
        while (system->cpu.wfe) {
            system->cpu.wfe = false;
            run_next_event(&system->sched);
        }
    }
    system->frame_complete = false;
}