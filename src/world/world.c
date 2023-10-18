#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "world.h"
#include "../game/player.h"
#include "../game/util.h"

struct World world;

int chunkIndex(int x, int y) {
   return (x + y * (GEN_LENGTH));
}

static _ivec2 getArrayIndexForWorldPos(ivec2 worldPos) {

    return (_ivec2) {
        wrap(player.arrIndex[0] + worldPos[0] - 3, WINDOW_SIZE),
        wrap(player.arrIndex[1] + worldPos[1] - 3, WINDOW_SIZE)
    };
}


static void genWorldChunk(ivec2 chunkPos, ivec2 arrayPos) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    initChunk(chunk, (ivec2){(chunkPos[0] - RENDER_DISTANCE) * CHUNK_SIZE_X, (chunkPos[1] - RENDER_DISTANCE) * CHUNK_SIZE_Z});
    genChunk(chunk);

    if (chunkIndex(arrayPos[0], arrayPos[1]) > GEN_AREA) {
        ERROR_IMSG("world.chunks array overflow at index", _tod(getArrayIndexForWorldPos(arrayPos), WINDOW_SIZE));
    }

    world.chunks[_tod(getArrayIndexForWorldPos(arrayPos), WINDOW_SIZE)] = *chunk;

    free(chunk);
}


static void meshWorldChunk(ivec2 arrayPos) {
    struct Chunk *chunkNeighbors = malloc(sizeof(struct Chunk) * 4);

    chunkNeighbors[RIGHT] = world.chunks[_tod(getArrayIndexForWorldPos((ivec2){arrayPos[0] + 1, arrayPos[1]}), WINDOW_SIZE)];
    chunkNeighbors[LEFT] = world.chunks[_tod(getArrayIndexForWorldPos((ivec2){arrayPos[0] - 1, arrayPos[1]}), WINDOW_SIZE)];
    chunkNeighbors[FRONT] = world.chunks[_tod(getArrayIndexForWorldPos((ivec2){arrayPos[0], arrayPos[1] + 1}), WINDOW_SIZE)];
    chunkNeighbors[BACK] = world.chunks[_tod(getArrayIndexForWorldPos((ivec2){arrayPos[0], arrayPos[1] - 1}), WINDOW_SIZE)];

    //for (int i = 0; i < 4; i++) chunkNeighbors[i].isNull = true;

    meshChunk(&world.chunks[_tod(getArrayIndexForWorldPos(arrayPos), WINDOW_SIZE)], chunkNeighbors);
    bindChunk(&world.chunks[_tod(getArrayIndexForWorldPos(arrayPos), WINDOW_SIZE)]);

    //free(chunkNeighbors);
}

void initWorld() {
    world.chunks = malloc(GEN_AREA * sizeof(struct Chunk));
    world.distToLeft = GEN_LENGTH / 2;
    world.distToRight = GEN_LENGTH - world.distToLeft - 1;

    glm_ivec2_copy((ivec2){0, 0}, world.oldPosition);

    for (int x = 3 - world.distToLeft; x <= 3 + world.distToRight; x++) {
        for (int y = 3 - world.distToLeft; y <= 3 + world.distToRight; y++) {
            genWorldChunk((ivec2){x, y}, (ivec2){x, y});
        }
    }

    // Meshing chunks surrounded by other chunks; see world.h:18
    for (int x = 3 - world.distToLeft; x <= 3 + world.distToRight; x++) {
        for (int y = 3 - world.distToLeft; y <= 3 + world.distToRight; y++) {
            meshWorldChunk((ivec2){x, y});
        }
    }

    LOG("World loaded!");
}

void moveWorld(ivec2 newPosition) {
    ivec2 result;
    glm_ivec2_sub(newPosition, world.oldPosition, result);
    glm_ivec2_copy(newPosition, world.oldPosition);

    /*if (result[0] == 1 && result[1] == 0) { // (+1, 0) Move right
        for (int x = 0; x < RENDER_LENGTH+1; x++) {
            for (int z = 0; z < RENDER_LENGTH+1; z++) {
                world.chunks[chunkIndex(x, z)] = world.chunks[chunkIndex(x + 1, z)];
            }
        }

        for (int z = 0; z < RENDER_LENGTH+1; z++) {
            genWorldChunk((ivec2){RENDER_LENGTH + 1 + newPosition[0], z + newPosition[1]}, (ivec2){RENDER_LENGTH + 1, z});

            if (z > 0 && z < RENDER_LENGTH+1) {
                meshWorldChunk((ivec2){RENDER_LENGTH, z});
            }
        }
    } else if (result[0] == -1 && result[1] == 0) { // (-1, 0) Move left
        for (int x = RENDER_LENGTH+1; x > 0; x--) {
            for (int z = 0; z < RENDER_LENGTH+1; z++) {
                world.chunks[chunkIndex(x, z)] = world.chunks[chunkIndex(x - 1, z)];
            }
        }

        for (int z = RENDER_LENGTH+1; z > 0; z--) {
            genWorldChunk((ivec2){newPosition[0], z + newPosition[1]}, (ivec2){0, z});

            if (z > 0 && z < RENDER_LENGTH+1) {
                meshWorldChunk((ivec2){1, z});
            }
        }
    } else if (result[0] == 0 && result[1] == 1) { // (0, 1)
        for (int x = 0; x < RENDER_LENGTH+1; x++) {
            for (int z = 0; z < RENDER_LENGTH+1; z++) {
                world.chunks[chunkIndex(x, z)] = world.chunks[chunkIndex(x, z + 1)];
            }
        }

        for (int x = 0; x < RENDER_LENGTH+1; x++) {
            genWorldChunk((ivec2){x + newPosition[0], RENDER_LENGTH + 1 + newPosition[1]}, (ivec2){x, RENDER_LENGTH + 1});

            if (x > 0 && x < RENDER_LENGTH+1) {
                meshWorldChunk((ivec2){x, RENDER_LENGTH});
            }
        }
    } else if (result[0] == 0 && result[1] == -1) { // (0, -1)
        for (int z = RENDER_LENGTH+1; z > 0; z--) {
            for (int x = 0; x < RENDER_LENGTH+1; x++) {
                world.chunks[chunkIndex(x, z)] = world.chunks[chunkIndex(x, z - 1)];
            }
        }

        for (int x = RENDER_LENGTH+1; x > 0; x--) {
            genWorldChunk((ivec2){x + newPosition[0], newPosition[1]}, (ivec2){x, 0});

            if (x > 0 && x < RENDER_LENGTH+1) {
                meshWorldChunk((ivec2){x, 1});
            }
        } 
    }*/
}


// Called in update function
void renderWorld(struct Shader shader) {

    for (int x = 3 - world.distToLeft; x <= 3 + world.distToRight; x++) {
        for (int y = 3 - world.distToLeft; y <= 3 + world.distToRight; y++) {
            renderChunk(&world.chunks[_tod(getArrayIndexForWorldPos((ivec2){x, y}), WINDOW_SIZE)], shader);
        }
    }
}

void destroyWorld() {
    for (int x = 0; x < GEN_LENGTH; x++) {
        for (int z = 0; z < GEN_LENGTH; z++) {
            destroyChunk(&world.chunks[chunkIndex(x, z)]);
        }
    }
}

void destroyBlock(ivec2 chunkPos, ivec3 blockPos) {
    world.chunks[chunkIndex(chunkPos[0], chunkPos[1])].blocks[blockIndex(blockPos[0], blockPos[1], blockPos[2])].id = 1;
}