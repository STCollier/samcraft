#ifndef WORLD_H
#define WORLD_H

#define RENDER_DISTANCE 6
#define RENDER_SIZE RENDER_DISTANCE * 2

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "../game/shader.h"
#include "../game/camera.h"
#include "../game/slidingwindow.h"
#include "chunk.h"

struct World {
    struct SlidingWindow *data;
};

void initWorld();
void loadWorld();
void renderWorld(struct Shader shader);
void destroyWorld();
//void destroyBlock(ivec2 chunkPos, ivec3 blockPos);

extern struct World world;

#endif