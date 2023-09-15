#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "world.h"
#include "../game/util.h"

struct World world;

static void loadWorldChunk(int offsetX, int offsetZ) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    initChunk(chunk, (vec3){(offsetX * CHUNK_SIZE_X) - RENDER_DISTANCE/2*CHUNK_SIZE_X, -CHUNK_SIZE_Y+64, (offsetZ * CHUNK_SIZE_Z) - RENDER_DISTANCE/2*CHUNK_SIZE_Z});

    world.chunks[offsetX][offsetZ] = *chunk;
}

// Called Once
void loadWorld() {
    struct Chunk *chunkNeighbors = malloc(sizeof(struct Chunk) * 4);

    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int z = 0; z < RENDER_DISTANCE; z++) {
            loadWorldChunk(x, z);
        }
    }

    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int z = 0; z < RENDER_DISTANCE; z++) {
            if (x+1 >= RENDER_DISTANCE) chunkNeighbors[RIGHT] = world.chunks[x][z];
            else chunkNeighbors[RIGHT] = world.chunks[x+1][z];

            if (x-1 < 0) chunkNeighbors[LEFT] = world.chunks[x][z];
            else chunkNeighbors[LEFT] = world.chunks[x-1][z];

            if (z+1 >= RENDER_DISTANCE) chunkNeighbors[FRONT] = world.chunks[x][z];
            else chunkNeighbors[FRONT] = world.chunks[x][z+1];

            if (z-1 < 0) chunkNeighbors[BACK] = world.chunks[x][z];
            else chunkNeighbors[BACK] = world.chunks[x][z-1];

            constructChunkMesh(&world.chunks[x][z], chunkNeighbors);
            loadChunk(&world.chunks[x][z]);
        }
    }

    free(chunkNeighbors);

    LOG("World loaded!");
}

// Called in update function
void renderWorld(struct Shader shader) {
    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int y = 0; y < RENDER_DISTANCE; y++) {
            renderChunk(&world.chunks[x][y], shader);
        }
    }
}

void destroyWorld() {
    /*for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int y = 0; y < RENDER_DISTANCE; y++) {
            destroyChunk(&world.chunks[x][y]);
        }
    }*/
}