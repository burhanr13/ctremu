#ifndef SERVICE_DATA_H
#define SERVICE_DATA_H

#include "thread.h"
#include "services/apt.h"
#include "services/dsp.h"
#include "services/gsp.h"
#include "services/hid.h"

typedef struct {

    KSemaphore notif_sem;

    APTData apt;
    GSPData gsp;
    HIDData hid;
    DSPData dsp;

} ServiceData;

#endif