#include "../func/player.h"

#ifndef GLOBALS_H
#define GLOBALS_H

#include "../util/common.h"

struct Globals {
    bool debug;
    int FOV;
    struct PlayerSpeed playerSpeed;
    float reach;
    float mouseSensitivity;
    int renderRadius;
    int threads;
};

void globals_init();

extern struct Globals globals;

#endif