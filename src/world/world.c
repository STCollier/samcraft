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
        wrap(player.arrIndex[0] + worldPos[0] - player.chunkPos[0], WINDOW_SIZE),
        wrap(player.arrIndex[1] + worldPos[1] - player.chunkPos[1], WINDOW_SIZE)
    };
}

static void debug() {
    printf("================================\n");
    printf("player index = (%d, %d)\n", player.arrIndex[0], player.arrIndex[1]);
    printf("player pos   = (%d, %d)\n", player.chunkPos[0], player.chunkPos[1]);
    printf("array in memory:\n");
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {

            _ivec2 index = (_ivec2){x, y};
            int x = world.chunks[_tod(index, WINDOW_SIZE)].worldPos[0];
            int y = world.chunks[_tod(index, WINDOW_SIZE)].worldPos[1];

            printf("(%d, %d) ", x, y);
        }
        puts("");
    }

    printf("chunks rendered:\n");
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {
            ivec2 index;
            index[0] = x;
            index[1] = y;

            int x = world.chunks[_tod(getArrayIndexForWorldPos(index), WINDOW_SIZE)].worldPos[0];
            int y = world.chunks[_tod(getArrayIndexForWorldPos(index), WINDOW_SIZE)].worldPos[1];

            printf("(%d, %d) ", x, y);
        }
        puts("");
    }
    printf("================================\n");
}

static void genWorldChunk(ivec2 chunkPos, int arrayIndex) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    initChunk(chunk, (ivec2){(chunkPos[0]) * CHUNK_SIZE_X, (chunkPos[1]) * CHUNK_SIZE_Z});
    genChunk(chunk);

    if (arrayIndex > GEN_AREA) {
        ERROR_IMSG("world.chunks array overflow at index", arrayIndex);
    }

    world.chunks[arrayIndex] = *chunk;

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

    for (int y = player.chunkPos[1] - world.distToLeft; y <= player.chunkPos[1] + world.distToRight; y++) {
        for (int x = player.chunkPos[0] - world.distToLeft; x <= player.chunkPos[0] + world.distToRight; x++) {
            ivec2 arrayPos = {x, y};
            int arrayIndex = _tod(getArrayIndexForWorldPos(arrayPos), WINDOW_SIZE);

            genWorldChunk((ivec2){x, y}, arrayIndex);
        }
    }

    // Meshing chunks surrounded by other chunks; see world.h:18
    for (int y = player.chunkPos[1] - world.distToLeft; y <= player.chunkPos[1] + world.distToRight; y++) {
        for (int x = player.chunkPos[0] - world.distToLeft; x <= player.chunkPos[0] + world.distToRight; x++) {
            meshWorldChunk((ivec2){x, y});
        }
    }
    debug();
    LOG("World loaded!");
}

void moveWorld(ivec2 newPosition) {
    ivec2 result;
    glm_ivec2_sub(newPosition, world.oldPosition, result);
    glm_ivec2_copy(newPosition, world.oldPosition);

    
    if (result[0] == -1 && result[1] == 0) { // (-1, 0) Move left
        // calculate the index in memory where you need to unload the old chunk and load the new one
        int indicesToSwap[GEN_LENGTH];
        ivec2 positionsToLoad[GEN_LENGTH];
        

        for (int i = 0; i < GEN_LENGTH; i++) {
            indicesToSwap[i] = _tod((_ivec2){
                wrap(player.arrIndex[0] + world.distToRight, WINDOW_SIZE),
                i
            }, WINDOW_SIZE);
        }

        int indicesToMesh[GEN_LENGTH];
        ivec2 positionsToMesh[GEN_LENGTH];

        // we could just store an array of size RENDER_LENGTH, but we're
        // going to use size GEN_LENGTH and ignore the first and last indices
        // so i lines up with position in the array
        for (int i = 1; i < GEN_LENGTH-1; i++) {
            indicesToMesh[i] = _tod((_ivec2){
                wrap(player.arrIndex[0] - world.distToLeft + 1, WINDOW_SIZE),
                i
            }, WINDOW_SIZE);
        }

        // subtract one from position and index, wrapping if necessary
        player.arrIndex[0] = wrap(player.arrIndex[0] - 1, WINDOW_SIZE);
        
        // calculate which world position to load
         for (int y = player.chunkPos[1] - world.distToLeft; y <= player.chunkPos[1] + world.distToRight; y++) {
            ivec2 worldPos;
            worldPos[0] = player.chunkPos[0] - world.distToLeft;
            worldPos[1] = y;
            glm_ivec2_copy(worldPos, positionsToLoad[getArrayIndexForWorldPos(worldPos).y]);
        }

        for (int y = player.chunkPos[1] - world.distToLeft; y <= player.chunkPos[1] + world.distToRight; y++) {
            printf("%d\n", y);

            ivec2 worldPos;
            worldPos[0] = player.chunkPos[0] - world.distToLeft + 1;
            worldPos[1] = y;
            glm_ivec2_copy(worldPos, positionsToMesh[getArrayIndexForWorldPos(worldPos).y]);
        }
        
        // perform load and unload
        for (int i = 0; i < GEN_LENGTH; i++) {
            printf("POSITION: (%d %d)\n", positionsToLoad[i][0], positionsToLoad[i][1]);

           genWorldChunk(positionsToLoad[i], indicesToSwap[i]);

        }

        for (int i = 1; i < GEN_LENGTH - 1; i++) {
            meshWorldChunk(positionsToMesh[i]);
        }

        debug();
    }
}


// Called in update function
void renderWorld(struct Shader shader) {
    for (int y = player.chunkPos[1] - world.distToLeft; y <= player.chunkPos[1] + world.distToRight; y++) {
        for (int x = player.chunkPos[0] - world.distToLeft; x <= player.chunkPos[0] + world.distToRight; x++) {
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