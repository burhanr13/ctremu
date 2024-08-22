#ifndef SVC_DEFS_H
#define SVC_DEFS_H

enum {
    MEMOP_ALLOC = 3,
    MEMOP_MASK = 0xff,

    MEMOP_REGMASK = 0xf00,

    MEMOP_LINEAR = 0x10000,
};

enum {
    RES_MEMORY = 1,
};

#endif
