#ifndef CPU_H
#define CPU_H

#include "3ds.h"
#include "arm/arm_core.h"
#include "types.h"

void cpu_init(HLE3DS* system);
void cpu_free(HLE3DS* system);

bool cpu_run(HLE3DS* system, int cycles);

u32 cpu_read8(HLE3DS* system, u32 addr, bool sx);
u32 cpu_read16(HLE3DS* system, u32 addr, bool sx);
u32 cpu_read32(HLE3DS* system, u32 addr);

void cpu_write8(HLE3DS* system, u32 addr, u8 b);
void cpu_write16(HLE3DS* system, u32 addr, u16 h);
void cpu_write32(HLE3DS* system, u32 addr, u32 w);

u16 cpu_fetch16(HLE3DS* system, u32 addr);
u32 cpu_fetch32(HLE3DS* system, u32 addr);

void cpu_handle_svc(HLE3DS* system, u32 num);

u32 cp15_read(HLE3DS* system, u32 cn, u32 cm, u32 cp);
void cp15_write(HLE3DS* system, u32 cn, u32 cm, u32 cp, u32 data);

#endif
