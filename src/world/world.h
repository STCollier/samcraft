#ifndef WORLD_H
#define WORLD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"
#include "cglm/call.h"
#include "uthash/uthash.h"

#include <stdbool.h>
#include <string.h>

#include "../engine/shader.h"
#include "../engine/camera.h"
#include "../engine/util.h"
#include "../engine/types.h"
#include "chunk.h"
#include "block.h"

struct World {
    struct Chunk *chunks;
    int renderRadius;
    int chunkRenderDepth;
};

int world_hashChunk(ivec2s position);
ivec2s world_unhashChunk(int hash);

void world_addChunk(ivec3 position);
struct Chunk *world_getChunk(ivec3 position);
void world_meshChunk(ivec3 position);

void world_init(int renderRadius);
void world_render(shader_t shader);

extern struct World world;

#endif