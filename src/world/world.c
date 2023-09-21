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

    initChunk(chunk, (vec3){(offsetX * CHUNK_SIZE_X), 0, (offsetZ * CHUNK_SIZE_Z)});

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
            if (x+1 >= RENDER_DISTANCE) chunkNeighbors[RIGHT] = world.chunks[chunkIndex(RENDER_DISTANCE - 1, z)];
            else chunkNeighbors[RIGHT] = world.chunks[chunkIndex(x + 1, z)];

            if (x-1 < 0) chunkNeighbors[LEFT] = world.chunks[chunkIndex(0, z)];
            else chunkNeighbors[LEFT] = world.chunks[chunkIndex(x - 1, z)];

            if (z+1 >= RENDER_DISTANCE) chunkNeighbors[FRONT] = world.chunks[chunkIndex(x, RENDER_DISTANCE-1)];
            else chunkNeighbors[FRONT] = world.chunks[chunkIndex(x, z + 1)];

            if (z-1 < 0) chunkNeighbors[BACK] = world.chunks[chunkIndex(x, 0)];
            else chunkNeighbors[BACK] = world.chunks[chunkIndex(x, z - 1)];

            if (x == 0 && z == 1) {
                chunkNeighbors[LEFT] = world.chunks[chunkIndex(0, 1)];
                chunkNeighbors[BACK] = world.chunks[chunkIndex(0, 1)];
            } else if (x == 1 && z == 0) {
                chunkNeighbors[LEFT] = world.chunks[chunkIndex(1, 0)];
                chunkNeighbors[BACK] = world.chunks[chunkIndex(1, 0)];
            }

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