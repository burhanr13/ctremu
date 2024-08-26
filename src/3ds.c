#include "3ds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "emulator_state.h"
#include "loader.h"
#include "svc_types.h"

void x3ds_init(X3DS* system, char* romfile) {
    memset(system, 0, sizeof *system);

    cpu_init(system);

    x3ds_memory_init(system);

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

    x3ds_vmalloc(system, STACK_BASE - STACK_SIZE, STACK_SIZE, PERM_RW, MEMST_PRIVATE);

    x3ds_vmalloc(system, CONFIG_MEM, PAGE_SIZE, PERM_R, MEMST_STATIC);
    x3ds_vmalloc(system, TLS_BASE, TLS_SIZE * MAX_RES, PERM_RW, MEMST_PRIVATE);

    init_services(system);

    thread_init(system, entrypoint);
}

void x3ds_destroy(X3DS* system) {
    cpu_free(system);

    x3ds_memory_destroy(system);
}

void x3ds_run_frame(X3DS* system) {
    if (!cpu_run(system, CPU_CLK / FPS)) {
        lerror("deadlock");
        exit(1);
    }
}