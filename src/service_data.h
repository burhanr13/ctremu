#ifndef SERVICE_DATA_H
#define SERVICE_DATA_H

#include "services/gsp.h"
#include "services/hid.h"

typedef struct {

    GSPData gsp;
    HIDData hid;

} ServiceData;

#endif