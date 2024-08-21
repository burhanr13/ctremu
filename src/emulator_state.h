#ifndef EMULATOR_STATE_H
#define EMULATOR_STATE_H

#include "3ds.h"
#include "types.h"

typedef struct {
    char* romfile;
    char* romfilenodir;

    bool initialized;
    bool running;
    bool uncap;
    bool bootbios;
    bool pause;
    bool mute;

    N3DS system;

} EmulatorState;

extern EmulatorState ctremu;

#endif