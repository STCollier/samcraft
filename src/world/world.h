#ifndef WORLD_H
#define WORLD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <pthread.h>
#include "cglm/cglm.h"
#include "cglm/call.h"
#include "uthash/uthash.h"

#include <stdbool.h>
#include <string.h>

#include "../engine/shader.h"
#include "../engine/camera.h"
#include "../engine/util.h"
#include "../engine/types.h"
#include "../engine/thpool.h"
#include "chunk.h"
#include "block.h"

struct ChunkList {
    struct Chunk **chunks;
    size_t size, capacity, index, tick;
};

struct ChunkQueue {
    struct ChunkList toGenerate;
    struct ChunkList toMesh;
    size_t passesPerFrame;
    bool queuesComplete;
};

enum ChunkQueueState {
    GENERATE, MESH, BIND
};

struct World {
    struct Chunk *chunks;
    struct ChunkQueue chunkQueue;
    int renderRadius, renderHeight;
};

void world_addChunk(ivec3 position);
struct Chunk *world_getChunk(ivec3 position);
void world_meshChunk(ivec3 position);
void world_remeshChunk(ivec3 position);

void world_init(int renderRadius);
void world_render(shader_t shader, threadpool thpool);

extern struct World world;

#endif