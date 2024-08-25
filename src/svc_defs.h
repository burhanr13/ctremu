#ifndef SVC_DEFS_H
#define SVC_DEFS_H

enum {
    MEMOP_ALLOC = 3,
    MEMOP_MASK = 0xff,

    MEMOP_REGMASK = 0xf00,

    MEMOP_LINEAR = 0x10000,
};

enum {
    PERM_R = BIT(0),
    PERM_W = BIT(1),
    PERM_X = BIT(2),
    PERM_RW = PERM_R | PERM_W,
    PERM_RX = PERM_R | PERM_X,
};

enum {
    MEMST_FREE = 0,
    MEMST_STATIC = 3,
    MEMST_CODE = 4,
    MEMST_PRIVATE = 5,
    MEMST_SHARED = 6,
};

enum {
    RES_MEMORY = 1,
};

#define IPC_CMD_OFF 0x80

#endif
