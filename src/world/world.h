#ifndef WORLD_H
#define WORLD_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "../game/shader.h"
#include "../game/camera.h"
#include "chunk.h"

// How many chunks will be rendered
#define RENDER_DISTANCE 8
#define RENDER_LENGTH (RENDER_DISTANCE * 2)
#define RENDER_AREA RENDER_LENGTH * RENDER_LENGTH // render length ^ 2

/*
How many chunks will be generated (render distance + 2, accounting for chunk borders on edge of rendering)
    g g g g g
    g # # # g
    g # # # g
    g # # # g
    g g g g g
*/

//#define GEN_DISTANCE (RENDER_DISTANCE + 1)
#define GEN_LENGTH (RENDER_LENGTH + 2)
#define GEN_AREA GEN_LENGTH * GEN_LENGTH

// Size of sliding window, equivalent to the generation length
#define WINDOW_SIZE GEN_LENGTH

struct World {
    struct Chunk *chunks;
    ivec2 oldPosition;
};

void initWorld();
void moveWorld(ivec2 newPosition);
void renderWorld(struct Shader shader);
//void destroyBlock(ivec2 chunkPos, ivec3 blockPos);

extern struct World world;

#endif