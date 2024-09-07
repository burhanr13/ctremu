#include "emulator.h"

#include <SDL2/SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arm/arm.h"
#include "arm/thumb.h"
#include "emulator_state.h"
#include "3ds.h"

EmulatorState ctremu;

const char usage[] = "ctremu [options] <romfile>\n"
                     "-h -- print help\n";

int emulator_init(int argc, char** argv) {
    read_args(argc, argv);
    if (!ctremu.romfile) {
        eprintf(usage);
        return -1;
    }

    arm_generate_lookup();
    thumb_generate_lookup();

    emulator_reset();

    ctremu.romfilenodir = strrchr(ctremu.romfile, '/');
    if (ctremu.romfilenodir) ctremu.romfilenodir++;
    else ctremu.romfilenodir = ctremu.romfile;
    return 0;
}

void emulator_quit() {
    hle3ds_destroy(&ctremu.system);
}

void emulator_reset() {
    if (ctremu.initialized) {
        hle3ds_destroy(&ctremu.system);
    }

    hle3ds_init(&ctremu.system, ctremu.romfile);

    ctremu.initialized = true;
}

void read_args(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (char* f = &argv[i][1]; *f; f++) {
                switch (*f) {
                    case 'h':
                        eprintf(usage);
                        exit(0);
                    default:
                        eprintf("Invalid argument\n");
                }
            }
        } else {
            ctremu.romfile = argv[i];
        }
    }
}

void hotkey_press(SDL_KeyCode key) {
    switch (key) {
        case SDLK_p:
            ctremu.pause = !ctremu.pause;
            break;
        case SDLK_m:
            ctremu.mute = !ctremu.mute;
            break;
        case SDLK_r:
            emulator_reset();
            ctremu.pause = false;
            break;
        case SDLK_TAB:
            ctremu.uncap = !ctremu.uncap;
            break;
        default:
            break;
    }
}
