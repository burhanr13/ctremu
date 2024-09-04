#ifndef VFP_H
#define VFP_H

#include "arm.h"
#include "arm_core.h"

void exec_vfp_data_proc(ArmCore* cpu, ArmInstr instr);
void exec_vfp_load_mem(ArmCore* cpu, ArmInstr instr, u32 addr);
void exec_vfp_store_mem(ArmCore* cpu, ArmInstr instr, u32 addr);
u32 exec_vfp_read(ArmCore* cpu, ArmInstr instr);
void exec_vfp_write(ArmCore* cpu, ArmInstr instr, u32 data);
u64 exec_vfp_read64(ArmCore* cpu, ArmInstr instr);
void exec_vfp_write64(ArmCore* cpu, ArmInstr instr, u64 data);

#endif
