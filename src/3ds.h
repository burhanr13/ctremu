#ifndef N3DS_H
#define N3DS_H

#include "cpu.h"

typedef struct _3DS {
    CPU cpu;

    u8* memory;
} N3DS;

void n3ds_init(N3DS* system, char* romfile);
void n3ds_destroy(N3DS* system);

void n3ds_run_frame(N3DS* system);

void n3ds_os_svc(N3DS* system, u32 num);

#endif
