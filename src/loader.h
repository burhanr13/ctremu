#ifndef LOADER_H
#define LOADER_H

#include "3ds.h"

u32 load_elf(HLE3DS* s, char* filename);
u32 load_ncsd(HLE3DS* s, char* filename);

u8* lzssrev_decompress(u8* in, u32 src_size, u32* dst_size);

#endif
