#ifndef APT_H
#define APT_H

#include "../srv.h"
#include "../svc_types.h"
#include "../thread.h"

typedef struct {
    KMutex lock;
    KEvent notif_event;
    KEvent resume_event;
} APTData;

DECL_PORT(apt);

#endif
