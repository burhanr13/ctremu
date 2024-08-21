#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "3ds.h"
#include "emulator.h"
#include "types.h"

char wintitle[200];

int main(int argc, char** argv) {

    if (emulator_init(argc, argv) < 0) return -1;

    Uint64 prev_time = SDL_GetPerformanceCounter();
    Uint64 prev_fps_update = prev_time;
    Uint64 prev_fps_frame = 0;
    const Uint64 frame_ticks = SDL_GetPerformanceFrequency() / 60;
    Uint64 frame = 0;

    ctremu.running = true;
    while (ctremu.running) {
        Uint64 cur_time;
        Uint64 elapsed;

        if (!(ctremu.pause)) {
            do {
                n3ds_run_frame(&ctremu.system);
                frame++;

                cur_time = SDL_GetPerformanceCounter();
                elapsed = cur_time - prev_time;
            } while (ctremu.uncap && elapsed < frame_ticks);
        }

        if (!ctremu.uncap) {
            cur_time = SDL_GetPerformanceCounter();
            elapsed = cur_time - prev_time;
            Sint64 wait = frame_ticks - elapsed;
            Sint64 waitMS =
                wait * 1000 / (Sint64) SDL_GetPerformanceFrequency();
            if (waitMS > 1 && !ctremu.uncap) {
                SDL_Delay(waitMS);
            }
        }
        cur_time = SDL_GetPerformanceCounter();
        elapsed = cur_time - prev_fps_update;
        if (elapsed >= SDL_GetPerformanceFrequency() / 2) {
            double fps = (double) SDL_GetPerformanceFrequency() *
                         (frame - prev_fps_frame) / elapsed;
            snprintf(wintitle, 199, "ctremu | %s | %.2lf FPS",
                     ctremu.romfilenodir, fps);
            prev_fps_update = cur_time;
            prev_fps_frame = frame;
        }
        prev_time = cur_time;
    }

    emulator_quit();

    return 0;
}
