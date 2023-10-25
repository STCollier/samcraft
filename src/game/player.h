#ifndef PLAYER_H
#define PLAYER_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "camera.h"
#include "../world/chunk.h"
#include <stdbool.h>

struct Player {
    vec3 position; // Floating point number representing position in [x, y, z] space
    ivec2 chunkPos; // Player's position in chunks, represented as a 2D integer vector [x, z]
    bool movedBetweenChunks;
};

void initPlayer();
void updatePlayer();

extern struct Player player;

#endif