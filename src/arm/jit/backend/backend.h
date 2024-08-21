#ifndef BACKEND_H
#define BACKEND_H

#ifdef __x86_64__
#include "backend_x86.h"
#define generate_code(ir, regalloc, cpu) generate_code_x86(ir, regalloc, cpu)
#define get_code(backend) get_code_x86(backend)
#define free_code(backend) free_code_x86(backend)
#define backend_disassemble(backend) backend_disassemble_x86(backend)
#else
#error("JIT not supported")
#endif

#endif