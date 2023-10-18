#ifndef PLAYER_H
#define PLAYER_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "camera.h"
#include "../world/chunk.h"
#include <stdbool.h>

struct Player {
    vec3 position;
    ivec2 chunkPos;
    ivec2 arrIndex;
    bool movedBetweenChunks;
};

void initPlayer();
void updatePlayer();

extern struct Player player;

#endif