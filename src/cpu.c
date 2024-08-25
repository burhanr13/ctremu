#include "cpu.h"

#include <string.h>

#include "3ds.h"
#include "arm/jit/jit.h"
#include "svc.h"

//#define CPULOG

void cpu_init(X3DS* system) {
    system->cpu.read8 = (void*) cpu_read8;
    system->cpu.read16 = (void*) cpu_read16;
    system->cpu.read32 = (void*) cpu_read32;
    system->cpu.write8 = (void*) cpu_write8;
    system->cpu.write16 = (void*) cpu_write16;
    system->cpu.write32 = (void*) cpu_write32;
    system->cpu.fetch16 = (void*) cpu_fetch16;
    system->cpu.fetch32 = (void*) cpu_fetch32;
    system->cpu.handle_svc = (void*) cpu_handle_svc;
    system->cpu.cp15_read = (void*) cp15_read;
    system->cpu.cp15_write = (void*) cp15_write;
}

void cpu_free(X3DS* system) {
    jit_free_all(&system->cpu);
}

void cpu_run(X3DS* system, int cycles) {
    system->cpu.cycles = cycles;
    while (system->cpu.cycles > 0) {
#ifdef CPULOG
        printf("executing at %08x\n", system->cpu.cur_instr_addr);
        cpu_print_state(&system->cpu);
#endif
        arm_exec_jit(&system->cpu);
    }
}

u32 cpu_read8(X3DS* system, u32 addr, bool sx) {
    if (sx) return *(s8*) PTR(addr);
    else return *(u8*) PTR(addr);
}
u32 cpu_read16(X3DS* system, u32 addr, bool sx) {
    if (sx) return *(s16*) PTR(addr);
    else return *(u16*) PTR(addr);
}
u32 cpu_read32(X3DS* system, u32 addr) {
    return *(u32*) PTR(addr);
}

void cpu_write8(X3DS* system, u32 addr, u8 b) {
    *(u8*) PTR(addr) = b;
}
void cpu_write16(X3DS* system, u32 addr, u16 h) {
    *(u16*) PTR(addr) = h;
}
void cpu_write32(X3DS* system, u32 addr, u32 w) {
    *(u32*) PTR(addr) = w;
}

u16 cpu_fetch16(X3DS* system, u32 addr) {
    return *(u16*) PTR(addr);
}
u32 cpu_fetch32(X3DS* system, u32 addr) {
    return *(u32*) PTR(addr);
}

void cpu_handle_svc(X3DS* system, u32 num) {
    x3ds_handle_svc(system, num);
}

u32 cp15_read(X3DS* system, u32 cn, u32 cm, u32 cp) {
    switch (cn) {
        case 13:
            switch (cm) {
                case 0:
                    switch (cp) {
                        case 3:
                            return TLS_BASE;
                    }
                    break;
            }
            break;
    }
    lwarn("unknown cp15 read: %d,%d,%d", cn, cm, cp);
    return 0;
}

void cp15_write(X3DS* system, u32 cn, u32 cm, u32 cp, u32 data) {
    switch (cn) {
        case 7:
            switch (cm) {
                case 10:
                    switch (cp) {
                        case 5:
                            return;
                    }
                    break;
            }
            break;
        case 13:
            switch (cm) {
                case 0:
                    switch (cp) {
                        case 3:
                            return;
                    }
                    break;
            }
            break;
    }
    lwarn("unknown cp15 write: %d,%d,%d", cn, cm, cp);
}