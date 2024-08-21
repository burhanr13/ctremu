#ifndef CPU_H
#define CPU_H

#include "arm/arm_core.h"
#include "types.h"

typedef struct _3DS N3DS;

typedef struct {
    ArmCore c;

    N3DS* master;
} CPU;

void cpu_init(CPU* cpu);
void cpu_free(CPU* cpu);

void cpu_run(CPU* cpu, int cycles);

u32 cpu_read8(CPU* cpu, u32 addr, bool sx);
u32 cpu_read16(CPU* cpu, u32 addr, bool sx);
u32 cpu_read32(CPU* cpu, u32 addr);

void cpu_write8(CPU* cpu, u32 addr, u8 b);
void cpu_write16(CPU* cpu, u32 addr, u16 h);
void cpu_write32(CPU* cpu, u32 addr, u32 w);

u16 cpu_fetch16(CPU* cpu, u32 addr);
u32 cpu_fetch32(CPU* cpu, u32 addr);

void cpu_handle_svc(CPU* cpu, u32 num);

u32 cp15_read(CPU* cpu, u32 cn, u32 cm, u32 cp);
void cp15_write(CPU* cpu, u32 cn, u32 cm, u32 cp, u32 data);

#endif
