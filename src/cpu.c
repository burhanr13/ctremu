#include "cpu.h"

#include "3ds.h"
#include "arm/jit/jit.h"
#include "svc.h"
#include "thread.h"

//#define CPULOG

void cpu_init(HLE3DS* system) {
    system->cpu.read8 = (void*) cpu_read8;
    system->cpu.read16 = (void*) cpu_read16;
    system->cpu.read32 = (void*) cpu_read32;
    system->cpu.write8 = (void*) cpu_write8;
    system->cpu.write16 = (void*) cpu_write16;
    system->cpu.write32 = (void*) cpu_write32;
    system->cpu.fetch16 = (void*) cpu_fetch16;
    system->cpu.fetch32 = (void*) cpu_fetch32;
    system->cpu.readf32 = (void*) cpu_readf32;
    system->cpu.readf64 = (void*) cpu_readf64;
    system->cpu.writef32 = (void*) cpu_writef32;
    system->cpu.writef64 = (void*) cpu_writef64;
    system->cpu.handle_svc = (void*) cpu_handle_svc;
    system->cpu.cp15_read = (void*) cp15_read;
    system->cpu.cp15_write = (void*) cp15_write;
}

void cpu_free(HLE3DS* system) {
    jit_free_all(&system->cpu);
}

bool cpu_run(HLE3DS* system, int cycles) {
    system->cpu.cycles = cycles;
    while (system->cpu.cycles > 0) {
#ifdef CPULOG
        printf("executing at %08x\n", system->cpu.pc);
        //cpu_print_state(&system->cpu);
#endif
        arm_exec_jit(&system->cpu);
        if (system->cpu.wfe) {
            system->cpu.wfe = false;
            return false;
        }
    }
    return true;
}

u32 cpu_read8(HLE3DS* system, u32 addr, bool sx) {
    if (sx) return *(s8*) PTR(addr);
    else return *(u8*) PTR(addr);
}
u32 cpu_read16(HLE3DS* system, u32 addr, bool sx) {
    if (sx) return *(s16*) PTR(addr);
    else return *(u16*) PTR(addr);
}
u32 cpu_read32(HLE3DS* system, u32 addr) {
    return *(u32*) PTR(addr);
}

void cpu_write8(HLE3DS* system, u32 addr, u8 b) {
    *(u8*) PTR(addr) = b;
}
void cpu_write16(HLE3DS* system, u32 addr, u16 h) {
    *(u16*) PTR(addr) = h;
}
void cpu_write32(HLE3DS* system, u32 addr, u32 w) {
    *(u32*) PTR(addr) = w;
}

u16 cpu_fetch16(HLE3DS* system, u32 addr) {
    return *(u16*) PTR(addr);
}
u32 cpu_fetch32(HLE3DS* system, u32 addr) {
    return *(u32*) PTR(addr);
}

float cpu_readf32(HLE3DS* system, u32 addr) {
    return *(float*) PTR(addr);
}

double cpu_readf64(HLE3DS* system, u32 addr) {
    return *(double*) PTR(addr);
}

void cpu_writef32(HLE3DS* system, u32 addr, float f) {
    *(float*) PTR(addr) = f;
}

void cpu_writef64(HLE3DS* system, u32 addr, double d) {
    *(double*) PTR(addr) = d;
}

void cpu_handle_svc(HLE3DS* system, u32 num) {
    hle3ds_handle_svc(system, num);
}

u32 cp15_read(HLE3DS* system, u32 cn, u32 cm, u32 cp) {
    switch (cn) {
        case 13:
            switch (cm) {
                case 0:
                    switch (cp) {
                        case 3:
                            return CUR_TLS;
                    }
                    break;
            }
            break;
    }
    lwarn("unknown cp15 read: %d,%d,%d", cn, cm, cp);
    return 0;
}

void cp15_write(HLE3DS* system, u32 cn, u32 cm, u32 cp, u32 data) {
    switch (cn) {
        case 7:
            switch (cm) {
                case 10:
                    switch (cp) {
                        case 4:
                            return;
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