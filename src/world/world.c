#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "world.h"
#include "../game/util.h"

struct World world;

static int chunkIndex(int x, int y) {
   return (x + y * WORLD_SIZE);
}

static void loadWorldChunk(int offsetX, int offsetZ) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    initChunk(chunk, (vec3){(offsetX * CHUNK_SIZE_X) - RENDER_DISTANCE/2*CHUNK_SIZE_X, -CHUNK_SIZE_Y+64, (offsetZ * CHUNK_SIZE_Z) - RENDER_DISTANCE/2*CHUNK_SIZE_Z});

    world.chunks[chunkIndex(offsetX, offsetZ)] = *chunk;

    free(chunk);
}

// Called Once
void loadWorld() {
    struct Chunk *chunkNeighbors = malloc(sizeof(struct Chunk) * 4);

    world.chunks = malloc(WORLD_SIZE * WORLD_SIZE * sizeof(struct Chunk));

    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int z = 0; z < RENDER_DISTANCE; z++) {
            loadWorldChunk(x, z);
        }
    }

    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int z = 0; z < RENDER_DISTANCE; z++) {
            if (x+1 >= RENDER_DISTANCE) chunkNeighbors[RIGHT].isNull = true;
            else chunkNeighbors[RIGHT] = world.chunks[chunkIndex(x + 1, z)];

            if (x-1 < 0) chunkNeighbors[LEFT].isNull = true;
            else chunkNeighbors[LEFT] = world.chunks[chunkIndex(x - 1, z)];

            if (z+1 >= RENDER_DISTANCE) chunkNeighbors[FRONT].isNull = true;
            else chunkNeighbors[FRONT] = world.chunks[chunkIndex(x, z + 1)];

            if (z-1 < 0) chunkNeighbors[BACK].isNull = true;
            else chunkNeighbors[BACK] = world.chunks[chunkIndex(x, z - 1)];

            constructChunkMesh(&world.chunks[chunkIndex(x, z)], chunkNeighbors);
            loadChunk(&world.chunks[chunkIndex(x, z)]);
        }
    }

    free(chunkNeighbors);

    LOG("World loaded!");
}

// Called in update function
void renderWorld(struct Shader shader) {
    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int y = 0; y < RENDER_DISTANCE; y++) {
            renderChunk(&world.chunks[chunkIndex(x, y)], shader);
        }
    }
}