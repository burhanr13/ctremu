#include "3ds.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cpu.h"
#include "emulator_state.h"
#include "loader.h"

void sigsegv_handler(int sig, siginfo_t* info, void* ucontext) {
    u8* addr = info->si_addr;
    if (ctremu.system.memory <= addr &&
        addr < ctremu.system.memory + BITL(32)) {
        eprintf("invalid 3DS memory access at %08x\n",
                addr - ctremu.system.memory);
        exit(1);
    }
    sigaction(SIGSEGV, &(struct sigaction){.sa_handler = SIG_DFL}, NULL);
}

void n3ds_init(N3DS* system, char* romfile) {

    cpu_init(&system->cpu);
    system->cpu.master = system;

    system->memory = mmap(NULL, BITL(32), PROT_NONE,
                          MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, -1, 0);
    if (system->memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    struct sigaction sa = {.sa_sigaction = sigsegv_handler,
                           .sa_flags = SA_SIGINFO};
    sigaction(SIGSEGV, &sa, NULL);

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

    system->cpu.c.cpsr.m = M_USER;
    system->cpu.c.pc = entrypoint;
    cpu_flush((ArmCore*) &system->cpu);
}

void n3ds_destroy(N3DS* system) {
    cpu_free(&system->cpu);

    sigaction(SIGSEGV, &(struct sigaction){.sa_handler = SIG_DFL}, NULL);
    munmap(system->memory, BITL(32));
}

void n3ds_run_frame(N3DS* system) {
    cpu_run(&system->cpu, 20000000);
}

void n3ds_os_svc(N3DS* system, u32 num) {
    switch (num) {
        default:
            eprintf("unknown svc: %x\n", num);
    }
}
