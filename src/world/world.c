#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "world.h"
#include "../game/util.h"

struct World world;

static void loadWorldChunk(vec2 chunkOffset) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    initChunk(chunk, (vec3){(chunkOffset[0] * CHUNK_SIZE_X) - RENDER_DISTANCE/2*CHUNK_SIZE_X, -CHUNK_SIZE_Y+32, (chunkOffset[1] * CHUNK_SIZE_Z) - RENDER_DISTANCE/2*CHUNK_SIZE_Z});
    constructChunkMesh(chunk);
    loadChunk(chunk);

    world.chunks[(int)chunkOffset[0]][(int)chunkOffset[1]] = *chunk;
}


void loadWorld() {
    for (int x = 0; x < RENDER_DISTANCE; x++) {
        for (int y = 0; y < RENDER_DISTANCE; y++) {
            loadWorldChunk((vec2){x, y});
        }
    }

    LOG("World loaded!");
}

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