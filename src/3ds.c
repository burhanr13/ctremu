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
        lerror("(FATAL) invalid 3DS memory access at %08x (pc near %08x)",
               addr - ctremu.system.memory, ctremu.system.cpu.c.pc);
        exit(1);
    }
    sigaction(SIGSEGV, &(struct sigaction){.sa_handler = SIG_DFL}, NULL);
}

void x3ds_init(X3DS* system, char* romfile) {

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
    system->free_memory = FCRAMSIZE;

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

    x3ds_mmap(system, STACK_BASE - STACK_SIZE, STACK_SIZE);

    x3ds_mmap(system, CONFIG_MEM, PAGE_SIZE);
    x3ds_mmap(system, TLS_BASE, PAGE_SIZE);

    system->cpu.c.cpsr.m = M_USER;
    system->cpu.c.sp = STACK_BASE;
    system->cpu.c.pc = entrypoint;
    cpu_flush((ArmCore*) &system->cpu);
}

void x3ds_destroy(X3DS* system) {
    cpu_free(&system->cpu);

    sigaction(SIGSEGV, &(struct sigaction){.sa_handler = SIG_DFL}, NULL);
    munmap(system->memory, BITL(32));
}

void x3ds_run_frame(X3DS* system) {
    cpu_run(&system->cpu, 20000000);
}

void* x3ds_mmap(X3DS* system, u32 addr, u32 size) {
    addr = addr & ~(PAGE_SIZE - 1);
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    void* ptr = mmap(&system->memory[addr], size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    system->free_memory -= size;
    linfo("mapped 3DS memory at %08x with size 0x%x", addr, size);
    return ptr;
}
