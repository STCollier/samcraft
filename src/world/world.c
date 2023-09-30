#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "world.h"
#include "../game/util.h"

struct World world;

static void loadWorldChunk(ivec2 offset, ivec2 arrayPos) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    initChunk(chunk, (ivec2){offset[0] * CHUNK_SIZE_X, offset[1] * CHUNK_SIZE_Z});

    setSWValue(world.data, arrayPos, chunk);
}

void initWorld() {
    world.data = malloc(sizeof(struct SlidingWindow)); // struct SlidingWindow *data;
    world.data->chunks = malloc(sizeof(struct Chunk) * WINDOW_SIZE * WINDOW_SIZE);
    glm_ivec2_copy((ivec2){0, 0}, world.data->minPosition);
    glm_ivec2_copy((ivec2){0, 0}, world.data->indexOffset);
}

void loadWorld() {
    struct Chunk chunkNeighbors[4];

    for (int x = 0; x < RENDER_SIZE; x++) {
        for (int z = 0; z < RENDER_SIZE; z++) {
            loadWorldChunk((ivec2){x - RENDER_DISTANCE, z - RENDER_DISTANCE}, (ivec2){x, z});
        }
    }

    for (int x = 0; x < RENDER_SIZE; x++) {
        for (int z = 0; z < RENDER_SIZE; z++) {
            if (x+1 >= RENDER_SIZE) chunkNeighbors[RIGHT] = *getSWValue(world.data, (ivec2){RENDER_SIZE - 1, z});
            else chunkNeighbors[RIGHT] = *getSWValue(world.data, (ivec2){x + 1, z});
            
            if (x-1 < 0) chunkNeighbors[LEFT] = *getSWValue(world.data, (ivec2){0, z});
            else chunkNeighbors[LEFT] = *getSWValue(world.data, (ivec2){x - 1, z});

            if (z+1 >= RENDER_SIZE) chunkNeighbors[FRONT] = *getSWValue(world.data, (ivec2){x, RENDER_SIZE - 1});
            else chunkNeighbors[FRONT] = *getSWValue(world.data, (ivec2){x, z + 1});

            if (z-1 < 0) chunkNeighbors[BACK] = *getSWValue(world.data, (ivec2){x, 0});
            else chunkNeighbors[BACK] = *getSWValue(world.data, (ivec2){x, z - 1});

            

            constructChunkMesh(getSWValue(world.data, (ivec2){x, z}), chunkNeighbors);
            loadChunk(getSWValue(world.data, (ivec2){x, z}));
        }
    }
}

// Called in update function
void renderWorld(struct Shader shader) {
    for (int x = 0; x < RENDER_SIZE; x++) {
        for (int z = 0; z < RENDER_SIZE; z++) {

            renderChunk(getSWValue(world.data, (ivec2){x, z}), shader);
        }
    }
}

void destroyWorld() {
    for (int x = 0; x < RENDER_SIZE; x++) {
        for (int z = 0; z < RENDER_SIZE; z++) {
            destroyChunk(getSWValue(world.data, (ivec2){x, z}));
        }
    }

    free(world.data);
}

/*void destroyBlock(ivec2 chunkPos, ivec3 blockPos) {
    world.chunks[chunkIndex(chunkPos[0], chunkPos[1])].blocks[blockIndex(blockPos[0], blockPos[1], blockPos[2])].id = 1;
}*/