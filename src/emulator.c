#include "emulator.h"

#include <SDL2/SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef USE_TFD
#include "tinyfiledialogs/tinyfiledialogs.h"
#endif

#include "3ds.h"
#include "emulator_state.h"
#include "services/hid.h"

bool g_infologs = false;
EmulatorState ctremu;

const char usage[] = "ctremu [options] <romfile>\n"
                     "-h -- print help\n"
                     "-l -- enable info logging\n"
                     "-sN -- upscale by N\n";

int emulator_init(int argc, char** argv) {
    ctremu.videoscale = 1;

    read_args(argc, argv);
    if (!ctremu.romfile) {
#ifdef USE_TFD
        const char* filetypes[] = {"*.3ds", "*.cci", "*.cxi", "*.app", "*.elf"};
        ctremu.romfile = tinyfd_openFileDialog(
            EMUNAME ": Open ROM", NULL, sizeof filetypes / sizeof filetypes[0],
            filetypes, "3DS ROMs", false);
        if (!ctremu.romfile) {
            eprintf(usage);
            return -1;
        }
#else
        eprintf(usage);
        return -1;
#endif
    }

    emulator_reset();

    ctremu.romfilenodir = strrchr(ctremu.romfile, '/');
    if (ctremu.romfilenodir) ctremu.romfilenodir++;
    else ctremu.romfilenodir = ctremu.romfile;
    ctremu.romfilenoext = strdup(ctremu.romfilenodir);

    char* c = strrchr(ctremu.romfilenoext, '.');
    if (c) *c = '\0';

    mkdir("system", S_IRWXU);
    mkdir("system/savedata", S_IRWXU);
    mkdir("system/extdata", S_IRWXU);
    mkdir("system/sdmc", S_IRWXU);

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
    char c;
    while ((c = getopt(argc, argv, "hls:")) != -1) {
        switch (c) {
            case 'l':
                g_infologs = true;
                break;
            case 's': {
                int scale = atoi(optarg);
                if (scale <= 0) eprintf("invalid scale factor");
                else ctremu.videoscale = scale;
                break;
            }
            case '?':
            case 'h':
            default:
                eprintf(usage);
                exit(0);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc >= 1) {
        ctremu.romfile = argv[0];
    }
}

void hotkey_press(SDL_KeyCode key) {
    switch (key) {
        case SDLK_F5:
            ctremu.pause = !ctremu.pause;
            break;
        case SDLK_F7:
            ctremu.freecam = !ctremu.freecam;
            glm_mat4_identity(ctremu.freecam_mtx);
            break;
        case SDLK_TAB:
            ctremu.uncap = !ctremu.uncap;
            break;
        default:
            break;
    }
}

void update_input(HLE3DS* s, SDL_GameController* controller) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    PadState btn = {};
    int cx = 0;
    int cy = 0;

    if (!ctremu.freecam) {
        btn.a = keys[SDL_SCANCODE_L];
        btn.b = keys[SDL_SCANCODE_K];
        btn.x = keys[SDL_SCANCODE_O];
        btn.y = keys[SDL_SCANCODE_I];
        btn.l = keys[SDL_SCANCODE_Q];
        btn.r = keys[SDL_SCANCODE_P];
        btn.start = keys[SDL_SCANCODE_RETURN];
        btn.select = keys[SDL_SCANCODE_RSHIFT];
        btn.up = keys[SDL_SCANCODE_UP];
        btn.down = keys[SDL_SCANCODE_DOWN];
        btn.left = keys[SDL_SCANCODE_LEFT];
        btn.right = keys[SDL_SCANCODE_RIGHT];

        cx = (keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * INT16_MAX;
        cy = (keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]) * INT16_MAX;
    }
    if (controller) {
        btn.a |=
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
        btn.b |=
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        btn.x |=
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
        btn.y |=
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
        btn.l |= SDL_GameControllerGetButton(
            controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        btn.r |= SDL_GameControllerGetButton(
            controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        btn.start |= SDL_GameControllerGetButton(controller,
                                                 SDL_CONTROLLER_BUTTON_START);
        btn.select |=
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
        btn.left |= SDL_GameControllerGetButton(
            controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        btn.right |= SDL_GameControllerGetButton(
            controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        btn.up |= SDL_GameControllerGetButton(controller,
                                              SDL_CONTROLLER_BUTTON_DPAD_UP);
        btn.down |= SDL_GameControllerGetButton(
            controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);

        int x =
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        if (abs(x) > abs(cx)) cx = x;
        int y =
            -SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        if (abs(y) > abs(cy)) cy = y;
    }

    btn.cup = cy > INT16_MAX / 2;
    btn.cdown = cy < INT16_MIN / 2;
    btn.cleft = cx < INT16_MIN / 2;
    btn.cright = cx > INT16_MAX / 2;

    hid_update_pad(s, btn.w, cx, cy);

    int x, y;
    bool pressed = SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT);

    if (pressed) {
        x -= (SCREEN_WIDTH - SCREEN_WIDTH_BOT) / 2 * ctremu.videoscale;
        x /= ctremu.videoscale;
        y -= SCREEN_HEIGHT * ctremu.videoscale;
        y /= ctremu.videoscale;
        if (x < 0 || x >= SCREEN_WIDTH_BOT || y < 0 || y >= SCREEN_HEIGHT) {
            hid_update_touch(s, 0, 0, false);
        } else {
            hid_update_touch(s, x, y, true);
        }
    } else {
        hid_update_touch(s, 0, 0, false);
    }
}

#define TRANSLATE_SPEED 5.0
#define ROTATE_SPEED 0.02

void update_input_freecam() {
    glUseProgram(ctremu.system.gpu.gl.gpuprogram);
    glUniform1i(
        glGetUniformLocation(ctremu.system.gpu.gl.gpuprogram, "freecam"),
        ctremu.freecam);

    if (!ctremu.freecam) return;

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    float speed = TRANSLATE_SPEED;
    if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]) speed /= 20;

    mat4 m;

    vec3 t = {};
    mat4 r = GLM_MAT4_IDENTITY_INIT;

    if (keys[SDL_SCANCODE_E]) {
        t[0] = -speed;
    }
    if (keys[SDL_SCANCODE_Q]) {
        t[0] = speed;
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        glm_rotate_make(r, -ROTATE_SPEED, GLM_YUP);
    }
    if (keys[SDL_SCANCODE_UP]) {
        glm_rotate_make(r, ROTATE_SPEED, GLM_YUP);
    }
    if (keys[SDL_SCANCODE_A]) {
        t[1] = -speed;
    }
    if (keys[SDL_SCANCODE_D]) {
        t[1] = speed;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
        glm_rotate_make(r, -ROTATE_SPEED, GLM_XUP);
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        glm_rotate_make(r, ROTATE_SPEED, GLM_XUP);
    }
    if (keys[SDL_SCANCODE_W]) {
        t[2] = speed;
    }
    if (keys[SDL_SCANCODE_S]) {
        t[2] = -speed;
    }

    glm_translate_make(m, t);
    glm_mat4_mul(m, ctremu.freecam_mtx, ctremu.freecam_mtx);
    glm_mat4_mul(r, ctremu.freecam_mtx, ctremu.freecam_mtx);
    glm_perspective(glm_rad(90), (float) SCREEN_HEIGHT / SCREEN_WIDTH, 0.1,
                    1000, ctremu.freecam_projmtx);
    glUniformMatrix4fv(
        glGetUniformLocation(ctremu.system.gpu.gl.gpuprogram, "freecam_view"),
        1, false, ctremu.freecam_mtx[0]);
    glUniformMatrix4fv(
        glGetUniformLocation(ctremu.system.gpu.gl.gpuprogram, "freecam_proj"),
        1, false, ctremu.freecam_projmtx[0]);
}