#ifndef WORLD_H
#define WORLD_H

#include "../engine/util/common.h"
#include "../engine/util/thpool.h"

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
    bool loaded;
    struct Chunk *chunks;
    struct ChunkQueue chunkQueue;
    int renderRadius, renderHeight;
};

uint8_t getBlockFromWorldPosition(int x, int y, int z);

void world_addChunk(ivec3 position);
struct Chunk *world_getChunk(ivec3 position);
void world_meshChunk(ivec3 position);
void world_remeshChunk(ivec3 position);

void world_init(int renderRadius);
void world_render(shader_t shader, threadpool thpool);

extern struct World world;

#endif