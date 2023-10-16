#ifndef WORLD_H
#define WORLD_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "../game/shader.h"
#include "../game/camera.h"
#include "chunk.h"

#define RENDER_DISTANCE 6
#define RENDER_LENGTH (RENDER_DISTANCE * 2)
#define RENDER_AREA RENDER_LENGTH * RENDER_LENGTH // render length ^ 2

struct World {
    struct Chunk *chunks;
    ivec2 oldPosition;
};

void initWorld();
void moveWorld(ivec2 newPosition);
void renderWorld(struct Shader shader);
void destroyBlock(ivec2 chunkPos, ivec3 blockPos);
void destroyWorld();

extern struct World world;

#endif