#ifndef CPU_H
#define CPU_H

#include "3ds.h"
#include "arm/arm_core.h"
#include "types.h"

void cpu_init(X3DS* system);
void cpu_free(X3DS* system);

void cpu_run(X3DS* system, int cycles);

u32 cpu_read8(X3DS* system, u32 addr, bool sx);
u32 cpu_read16(X3DS* system, u32 addr, bool sx);
u32 cpu_read32(X3DS* system, u32 addr);

void cpu_write8(X3DS* system, u32 addr, u8 b);
void cpu_write16(X3DS* system, u32 addr, u16 h);
void cpu_write32(X3DS* system, u32 addr, u32 w);

u16 cpu_fetch16(X3DS* system, u32 addr);
u32 cpu_fetch32(X3DS* system, u32 addr);

void cpu_handle_svc(X3DS* system, u32 num);

u32 cp15_read(X3DS* system, u32 cn, u32 cm, u32 cp);
void cp15_write(X3DS* system, u32 cn, u32 cm, u32 cp, u32 data);

#endif
