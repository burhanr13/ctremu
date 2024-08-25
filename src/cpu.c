#include "cpu.h"

#include <string.h>

#include "3ds.h"
#include "arm/jit/jit.h"
#include "svc.h"

void cpu_init(CPU* cpu) {
    memset(cpu, 0, sizeof *cpu);

    cpu->c.read8 = (void*)cpu_read8;
    cpu->c.read16 = (void*) cpu_read16;
    cpu->c.read32 = (void*) cpu_read32;
    cpu->c.write8 = (void*) cpu_write8;
    cpu->c.write16 = (void*) cpu_write16;
    cpu->c.write32 = (void*) cpu_write32;
    cpu->c.fetch16 = (void*) cpu_fetch16;
    cpu->c.fetch32 = (void*) cpu_fetch32;
    cpu->c.handle_svc = (void*) cpu_handle_svc;
    cpu->c.cp15_read = (void*) cp15_read;
    cpu->c.cp15_write = (void*) cp15_write;
}

void cpu_free(CPU* cpu) {
    jit_free_all((ArmCore*) cpu);
}

void cpu_run(CPU* cpu, int cycles) {
    cpu->c.cycles = cycles;
    while (cpu->c.cycles > 0) {
#ifdef CPULOG
        printf("executing at %08x\n", cpu->c.cur_instr_addr);
#endif
        arm_exec_jit((ArmCore*) cpu);
    }
}

u32 cpu_read8(CPU* cpu, u32 addr, bool sx) {
    void* realaddr = &cpu->master->memory[addr];
    if (sx) return *(s8*) realaddr;
    else return *(u8*) realaddr;
}
u32 cpu_read16(CPU* cpu, u32 addr, bool sx) {
    void* realaddr = &cpu->master->memory[addr];
    if (sx) return *(s16*) realaddr;
    else return *(u16*) realaddr;
}
u32 cpu_read32(CPU* cpu, u32 addr) {
    void* realaddr = &cpu->master->memory[addr];
    return *(u32*) realaddr;
}

void cpu_write8(CPU* cpu, u32 addr, u8 b) {
    void* realaddr = &cpu->master->memory[addr];
    *(u8*) realaddr = b;
}
void cpu_write16(CPU* cpu, u32 addr, u16 h) {
    void* realaddr = &cpu->master->memory[addr];
    *(u16*) realaddr = h;
}
void cpu_write32(CPU* cpu, u32 addr, u32 w) {
    void* realaddr = &cpu->master->memory[addr];
    *(u32*) realaddr = w;
}

u16 cpu_fetch16(CPU* cpu, u32 addr) {
    void* realaddr = &cpu->master->memory[addr];
    return *(u16*) realaddr;
}
u32 cpu_fetch32(CPU* cpu, u32 addr) {
    void* realaddr = &cpu->master->memory[addr];
    return *(u32*) realaddr;
}

void cpu_handle_svc(CPU* cpu, u32 num) {
    x3ds_handle_svc(cpu->master, num);
}

u32 cp15_read(CPU* cpu, u32 cn, u32 cm, u32 cp) {
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
    lwarn("unimpl cp15 read: %d,%d,%d", cn, cm, cp);
    return 0;
}

void cp15_write(CPU* cpu, u32 cn, u32 cm, u32 cp, u32 data) {
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
    lwarn("unimpl cp15 write: %d,%d,%d", cn, cm, cp);
}