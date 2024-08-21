#ifndef BACKEND_X86_H
#define BACKEND_X86_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ir.h"
#include "../jit.h"
#include "../register_allocator.h"
#include "../../arm_core.h"

void* generate_code_x86(IRBlock* ir, RegAllocation* regalloc, ArmCore* cpu);
JITFunc get_code_x86(void* backend);
void free_code_x86(void* backend);
void backend_disassemble_x86(void* backend);

#ifdef __cplusplus
}
#endif

#endif
