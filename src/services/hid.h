#ifndef HID_H
#define HID_H

#include "../svc_types.h"
#include "../srv.h"

enum {
    HIDEVENT_PAD0,
    HIDEVENT_PAD1,
    HIDEVENT_ACCEL,
    HIDEVENT_GYRO,
    HIDEVENT_DBGPAD,
    HIDEVENT_MAX
};

typedef struct {
    Handle memblock;
    Handle events[HIDEVENT_MAX];
} HIDData;

DECL_SRV(hid);

#endif
