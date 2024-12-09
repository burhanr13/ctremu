#ifndef EMULATOR_STATE_H
#define EMULATOR_STATE_H

#include <cglm/cglm.h>

#include "3ds.h"

typedef struct {
    char* romfile;
    char* romfilenodir;
    char* romfilenoext;

    bool initialized;
    bool running;
    bool uncap;
    bool pause;
    bool mute;

    HLE3DS system;

    int videoscale;

    bool freecam;
    mat4 freecam_mtx;
    mat4 freecam_projmtx;

} EmulatorState;

extern EmulatorState ctremu;

#endif