#ifndef HID_H
#define HID_H

#include "../memory.h"
#include "../srv.h"
#include "../svc_types.h"
#include "../thread.h"

enum {
    HIDEVENT_PAD0,
    HIDEVENT_PAD1,
    HIDEVENT_ACCEL,
    HIDEVENT_GYRO,
    HIDEVENT_DBGPAD,
    HIDEVENT_MAX
};

typedef struct {
    KSharedMem sharedmem;
    KEvent events[HIDEVENT_MAX];
} HIDData;

DECL_PORT(hid);

#endif
