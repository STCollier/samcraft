#ifndef WORLD_H
#define WORLD_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "../game/shader.h"
#include "../game/camera.h"
#include "chunk.h"

#define WORLD_SIZE 10 // # of chunks
#define RENDER_DISTANCE 6

struct World {
    struct Chunk *chunks;
};

void loadWorld();
void renderWorld(struct Shader shader);

extern struct World world;

#endif